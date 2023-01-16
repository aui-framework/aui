// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by alex2772 on 4/19/22.
//

#include "UnixIoThread.h"
#include "UnixEventFd.h"
#include "AUI/Thread/ACutoffSignal.h"
#include <AUI/Thread/IEventLoop.h>
#include <AUI/Thread/AFuture.h>
#include <sys/poll.h>

UnixIoThread& UnixIoThread::inst() noexcept {
    static UnixIoThread s;
    return s;
}

class MyEventLoop: public IEventLoop {
public:
    MyEventLoop(UnixIoThread& parent) : mParent(parent) {}

    ~MyEventLoop() override {

    }

    void notifyProcessMessages() override {
        mParent.mNotifyEvent.set();
    }

    void loop() override {
        for (;;) {
            AThread::processMessages();
            auto r = poll(mParent.mPollFd.data(), mParent.mPollFd.size(), -1);
            assert(r > 0);
            for (auto it = mParent.mPollFd.begin(); it != mParent.mPollFd.end() && r > 0; ++it) {
                if (it->revents) {
                    mParent.mCallbacks[it - mParent.mPollFd.begin()](it->revents);
                    r -= 1;
                    it->revents = 0;
                }
            }
        }
    }
private:
    UnixIoThread& mParent;
};

void UnixIoThread::registerCallback(int fd, int flags, Callback callback) noexcept {
    AFuture<> ft;
    mThread->enqueue([&, callback = std::move(callback)]() mutable {
        mCallbacks << std::move(callback);
        mPollFd << pollfd {
            fd, static_cast<short>(flags), 0
        };
        ft.supplyResult();
    });
    ft.wait();
}

void UnixIoThread::unregisterCallback(int fd) noexcept {
    AFuture<> ft;
    mThread->enqueue([&] {
        std::size_t index = 0;
        mPollFd.removeIf([&](const pollfd& p) {
            if (p.fd == fd) {
                mCallbacks.removeAt(index);
                return true;
            }
            ++index;
            return false;
        });
        ft.supplyResult();
    });
    ft.wait();
}

UnixIoThread::UnixIoThread() noexcept: mThread(_new<AThread>([&] {
    AThread::setName("AUI IO");
    MyEventLoop loop(*this);
    IEventLoop::Handle handle(&loop);
    AThread::current()->getCurrentEventLoop()->loop();
})) {
    mCallbacks << [&](int){
        mNotifyEvent.reset();
    };
    mPollFd << pollfd {
            mNotifyEvent.handle(),
            POLLIN
    };
    mThread->start();

    ACutoffSignal cv;
    mThread->enqueue([&] {
        cv.makeSignal();
    });
    cv.waitForSignal();
}
