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
// Created by alex2772 on 4/19/22.
//

#pragma once

#include <AUI/Thread/AThread.h>
#include <sys/poll.h>
#include "UnixEventFd.h"
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/EnumUtil.h>
#include <AUI/Util/ABitField.h>
#include <unordered_map>


#ifdef __linux
#include <sys/epoll.h>
AUI_ENUM_FLAG(UnixPollEvent) {
    IN  = EPOLLIN,
    OUT = EPOLLOUT,
};
#else
AUI_ENUM_FLAG(UnixPollEvent) {
    IN  = POLLIN,
    OUT = POLLOUT,
};
#endif


class API_AUI_CORE UnixIoThread {
public:
    using Callback = std::function<void(ABitField<UnixPollEvent> triggeredFlags)>;

    static UnixIoThread& inst() noexcept;

    void registerCallback(int fd, ABitField<UnixPollEvent> flags, Callback callback) noexcept;
    void unregisterCallback(int fd) noexcept;

private:
    friend class MyEventLoop;
    _<AThread> mThread;
    UnixEventFd mNotifyEvent;

#ifdef __linux
    int mEpollFd;
    struct FDInfo {
        struct CallbackEntry {
            ABitField<UnixPollEvent> mask;
            Callback callback;
        };
        AVector<CallbackEntry> callbacks;
    };
    AMutex mSync;
    std::unordered_map<int /* fd */, FDInfo> mFdInfo;
#else
    AVector<pollfd> mPollFd;
    AVector<Callback> mCallbacks;
#endif

    template<aui::invocable Callback>
    void executeOnIoThreadBlocking(Callback&& callback) {
        if (AThread::current() == mThread) {
            callback();
            return;
        }

        AFuture<> cs;
        mThread->enqueue([&] {
            cs.supplyResult();
        });
        cs.wait();
    }

    UnixIoThread() noexcept;
};




