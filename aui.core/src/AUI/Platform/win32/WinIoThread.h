#pragma once


#include "AUI/Thread/AThread.h"

class WinIoThread {
public:
    static WinIoThread& inst() noexcept;

    static void enqueue(std::function<void()> function) noexcept {
        inst().mThread->enqueue(std::move(function));
    }

private:
    _<AThread> mThread;

    WinIoThread() noexcept;
};


