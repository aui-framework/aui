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
// Created by alex2772 on 4/19/22.
//

#include "UnixIoThread.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/Thread/AThread.h"
#include "UnixEventFd.h"
#include <AUI/Thread/IEventLoop.h>
#include <sys/poll.h>
#include <AUI/Thread/AFuture.h>

UnixIoThread& UnixIoThread::inst() noexcept {
    static UnixIoThread s;
    return s;
}

#ifdef __linux

#include <sys/epoll.h>
#include <AUI/Platform/ErrorToException.h>


class MyEventLoop: public IEventLoop {
public:
    MyEventLoop(UnixIoThread& parent) : mParent(parent) {
    }

    ~MyEventLoop() override {

    }

    void notifyProcessMessages() override {
        mParent.mNotifyEvent.set();
    }

    void loop() override {
        for (;;) {
            AThread::processMessages();
            epoll_event events[1024];
            int count = epoll_wait(mParent.mEpollFd, events, std::size(events), -1);
            if (count < 0) {
                if (errno == EINTR) {
                    continue;
                }
                aui::impl::unix_based::lastErrorToException("epoll_wait failed");
            }
            std::unique_lock lock (mParent.mSync);
            for (int i = 0; i < count; ++i) {
                auto it = mParent.mFdInfo.find(events[i].data.fd);
                if (it == mParent.mFdInfo.end()) continue;
                for (const auto& callback : it->second.callbacks) {
                    if (!callback.mask.testAny(static_cast<UnixPollEvent>(events[i].events))) {
                        continue;
                    }
                    callback.callback(static_cast<UnixPollEvent>(events[i].events));
                }
            }
        }
    }
private:
    UnixIoThread& mParent;
};

void UnixIoThread::registerCallback(int fd, ABitField<UnixPollEvent> flags, Callback callback) noexcept {
    std::unique_lock lock(mSync);
    mFdInfo[fd].callbacks << FDInfo::CallbackEntry { flags, std::move(callback) };
    epoll_event e;
    e.events = static_cast<uint32_t>(flags.value());
    e.data.fd = fd;
    if (epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &e) == -1) {
        aui::impl::unix_based::lastErrorToException("epoll_ctl add failed");
    }
}

void UnixIoThread::unregisterCallback(int fd) noexcept {
    epoll_event e;
    e.events = 0xffffffff;
    e.data.fd = fd;
    if (epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, &e) == -1) {
        aui::impl::unix_based::lastErrorToException("epoll_ctl del+ failed");
    }
    mThread->enqueue([this, fd] {
        std::unique_lock lock (mSync);
        mFdInfo.erase(fd);
    });
}

UnixIoThread::UnixIoThread() noexcept: mThread(_new<AThread>([&] {
    AThread::setName("AUI IO");
    MyEventLoop loop(*this);
    IEventLoop::Handle handle(&loop);
    AThread::current()->getCurrentEventLoop()->loop();
})), mEpollFd(epoll_create1(0)) {
    registerCallback(mNotifyEvent.handle(), UnixPollEvent::IN, [this](ABitField<UnixPollEvent> f) {
        mNotifyEvent.reset();
    });

    mThread->start();
    AFuture<> cs;
    mThread->enqueue([&] {
        cs.supplyResult();
    });
    cs.wait();
}

#else

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
            AUI_ASSERT(r > 0);
            for (auto it = mParent.mPollFd.begin(); it != mParent.mPollFd.end() && r > 0; ++it) {
                if (it->revents) {
                    mParent.mCallbacks[it - mParent.mPollFd.begin()](static_cast<UnixPollEvent>(it->revents));
                    r -= 1;
                    it->revents = 0;
                }
            }
        }
    }
private:
    UnixIoThread& mParent;
};

void UnixIoThread::registerCallback(int fd, ABitField<UnixPollEvent> flags, Callback callback) noexcept {
    executeOnIoThreadBlocking([&]() mutable {
        mCallbacks << std::move(callback);
        mPollFd << pollfd {
            fd, static_cast<short>(flags.value()), 0
        };
    });
}

void UnixIoThread::unregisterCallback(int fd) noexcept {
    executeOnIoThreadBlocking([&] {
        std::size_t index = 0;
        mPollFd.removeIf([&](const pollfd& p) {
            if (p.fd == fd) {
                mCallbacks.removeAt(index);
                return true;
            }
            ++index;
            return false;
        });
    });
}

UnixIoThread::UnixIoThread() noexcept: mThread(_new<AThread>([&] {
    AThread::setName("AUI IO");
    MyEventLoop loop(*this);
    IEventLoop::Handle handle(&loop);
    AThread::current()->getCurrentEventLoop()->loop();
})) {
    mCallbacks << [&](ABitField<UnixPollEvent>){
        mNotifyEvent.reset();
    };
    mPollFd << pollfd {
            mNotifyEvent.handle(),
            POLLIN
    };
    mThread->start();

    AFuture<> cs;
    mThread->enqueue([&] {
        cs.supplyResult();
    });
    cs.wait();
}
#endif
