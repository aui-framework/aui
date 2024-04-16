// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
