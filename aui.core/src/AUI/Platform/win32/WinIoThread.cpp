//
// Created by Alex2772 on 4/19/2022.
//

#include "WinIoThread.h"
#include "AUI/Thread/ACutoffSignal.h"
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
        MyEventLoop(): mNotifyHandle(CreateEvent(nullptr, false, false, L"WinIoThread::MyEventLoop")) {
            assert(mNotifyHandle != nullptr);
        }

        ~MyEventLoop() override {
            CloseHandle(mNotifyHandle);
        }

        void notifyProcessMessages() override {
            auto r = SetEvent(mNotifyHandle);
            assert(r);
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
    ACutoffSignal cutoff;
    mThread->enqueue([&] {
        cutoff.makeSignal();
    });
    cutoff.waitForSignal();
}
