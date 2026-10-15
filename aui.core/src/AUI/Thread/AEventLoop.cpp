/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2772 on 9/5/22.
//

#include "AEventLoop.h"
#include "AThread.h"

AEventLoop::~AEventLoop() {

}

void AEventLoop::notifyProcessMessages() {
    std::unique_lock lock(mMutex);
    mNotified = true;
    mCV.notify_all();
}

void AEventLoop::loop() {
    mRunning = true;
    while (mRunning) {
        iteration();
    }
}

void AEventLoop::iteration() {
    std::unique_lock lock(mMutex);
    if (mNotified) {
        mNotified = false;
        lock.unlock();
        AThread::processMessages();
        return;
    }
    if (!AThread::current()->messageQueueEmpty()) {
        lock.unlock();
        AThread::processMessages();
        return;
    }
    mNotified = true;
    mCV.wait(lock);
    lock.unlock();
    AThread::processMessages();
}
