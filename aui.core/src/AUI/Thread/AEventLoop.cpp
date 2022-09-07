//
// Created by alex2772 on 9/5/22.
//

#include "AEventLoop.h"
#include "AThread.h"

AEventLoop::~AEventLoop() {

}

void AEventLoop::notifyProcessMessages() {
    std::unique_lock lock(mMutex);
    mNotified = true;
    mCV.notify_all();
}

void AEventLoop::loop() {
    mRunning = true;
    while (mRunning) {
        iteration();
    }
}

void AEventLoop::iteration() {
    AThread::processMessages();
    std::unique_lock lock(mMutex);
    if (!mNotified) {
        mNotified = false;
        mCV.wait(lock);
    }
}
