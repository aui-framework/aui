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
// Created by alex2772 on 4/19/22.
//

#pragma once

#include <AUI/Thread/AThread.h>
#include <sys/poll.h>
#include "UnixEventFd.h"
#include <AUI/Thread/AFuture.h>
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


/**
 * @brief Poll-based event loop to handle events of file descriptors.
 */
class API_AUI_CORE UnixIoThread {
public:
    using Callback = std::function<void(ABitField<UnixPollEvent> triggeredFlags)>;

    static UnixIoThread& inst() noexcept;

    void registerCallback(int fd, ABitField<UnixPollEvent> flags, Callback callback) noexcept;
    void unregisterCallback(int fd) noexcept;
    static const _<AThread>& thread() noexcept {
        return inst().mThread;
    }

private:
    friend class UnixIoEventLoop;
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
    AMessageQueue<> mMessageQueue;

    template<aui::invocable Callback>
    void executeOnIoThreadBlocking(Callback&& callback) {
        if (AThread::current() == mThread) {
            callback();
            return;
        }

        AFuture<> cs;
        mMessageQueue.enqueue([&] {
            callback();
            cs.supplyValue();
        });
        mNotifyEvent.set();
        cs.wait();
    }

    UnixIoThread() noexcept;
};




