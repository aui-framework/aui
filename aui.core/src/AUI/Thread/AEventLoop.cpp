//
// Created by alex2772 on 9/5/22.
//

#include "AEventLoop.h"
#include "AThread.h"

AEventLoop::~AEventLoop() {

}

void AEventLoop::notifyProcessMessages() {
    std::unique_lock lock(mMutex);
    mLoopFlag = true;
    mCV.notify_all();
}

void AEventLoop::loop() {
    for (;;) {
        AThread::processMessages();
        std::unique_lock lock(mMutex);
        if (!mLoopFlag) {
            mCV.wait(lock);
        }
    }
}
