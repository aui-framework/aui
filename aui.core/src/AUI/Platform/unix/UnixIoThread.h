//
// Created by alex2772 on 4/19/22.
//

#pragma once

#include <AUI/Thread/AThread.h>
#include <sys/poll.h>
#include "UnixEventFd.h"

class API_AUI_CORE UnixIoThread {
public:
    using Callback = std::function<void(int)>;

    static UnixIoThread& inst() noexcept;

    void registerCallback(int fd, int flags, Callback callback) noexcept;
    void unregisterCallback(int fd) noexcept;

private:
    friend class MyEventLoop;
    _<AThread> mThread;
    UnixEventFd mNotifyEvent;
    AVector<pollfd> mPollFd;
    AVector<Callback> mCallbacks;

    UnixIoThread() noexcept;
};



