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
