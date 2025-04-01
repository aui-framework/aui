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
// Created by Alex2772 on 4/19/2022.
//

#include "WinIoThread.h"
#include <AUI/Thread/IEventLoop.h>
#include <AUI/Thread/AFuture.h>
#include <Windows.h>

WinIoThread& WinIoThread::inst() noexcept {
    static WinIoThread t;
    return t;
}


namespace {
    class MyEventLoop: public IEventLoop {
    public:
        MyEventLoop(): mNotifyHandle(CreateEvent(nullptr, false, false, nullptr)) {
            AUI_ASSERT(mNotifyHandle != nullptr);
        }

        ~MyEventLoop() override {
            CloseHandle(mNotifyHandle);
        }

        void notifyProcessMessages() override {
            auto r = SetEvent(mNotifyHandle);
            AUI_ASSERT(r);
        }

        void loop() override {
            for (;;) {
                AThread::processMessages();
                WaitForSingleObjectEx(mNotifyHandle, INFINITE, true);
            }
        }
    private:
        HANDLE mNotifyHandle;
    };
}


WinIoThread::WinIoThread() noexcept: mThread(_new<AThread>([] {
    AThread::setName("AUI IO");
    MyEventLoop loop;
    IEventLoop::Handle handle(&loop);
    AThread::current()->getCurrentEventLoop()->loop();
})) {
    mThread->start();

    AFuture<> cs;
    mThread->enqueue([&] {
        cs.supplyValue();
    });
    cs.wait();
}
