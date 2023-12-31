// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
