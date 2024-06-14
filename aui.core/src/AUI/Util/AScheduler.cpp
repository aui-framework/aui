/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by Alex2772 on 9/21/2022.
//

#include "AScheduler.h"


AScheduler::AScheduler() {

}

bool AScheduler::iteration(ABitField<ASchedulerIteration> flag) {
    std::unique_lock lock(mSync);

    if (mTasks.empty()) {
        if (flag & ASchedulerIteration::DONT_BLOCK_INFINITELY) {
            return false;
        }
        mCV.wait(lock);
    }

    while (!mTasks.empty()) {
        auto now = currentTime();
        if (now < mTasks.front().executionTime) {
            if (flag & ASchedulerIteration::DONT_BLOCK_TIMED) {
                return false;
            }

            auto t = mTasks.front().executionTime;
            mCV.wait_until(lock, t);
            break;
        }
        auto callback = std::move(mTasks.front().callback);
        mTasks.pop_front();
        lock.unlock();
        callback();
        lock.lock();
    }

    return true;
}


void AScheduler::notifyProcessMessages() {
    mCV.notify_all();
}

void AScheduler::loop() {
    for (;;) {
        iteration();
    }
}

void AScheduler::removeTimer(const TimerHandle& t) {
    std::unique_lock lock(mSync);
    auto timer = t.lock();
    if (!timer) {
        return;
    }

    mTasks.erase(std::remove_if(mTasks.begin(), mTasks.end(), [&](const Task& rhs) {
        return rhs.timer.lock() == timer;
    }), mTasks.end());
    if (auto it = std::find(mTimers.begin(), mTimers.end(), timer); it != mTimers.end()) {
        mTimers.erase(it);
    }
    mCV.notify_all();
}
