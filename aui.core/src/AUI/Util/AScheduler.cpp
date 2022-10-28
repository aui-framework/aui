//
// Created by Alex2772 on 9/21/2022.
//

#include "AScheduler.h"


AScheduler::AScheduler() {

}

bool AScheduler::iteration(ABitField<ASchedulerIteration> flag) {
    std::unique_lock lock(mSync);

    if (mTasks.empty()) {
        if (flag & ASchedulerIteration::DONT_BLOCK_INFINITELY) {
            return false;
        }
        mCV.wait(lock);
    }

    while (!mTasks.empty()) {
        auto now = currentTime();
        if (now < mTasks.front().executionTime) {
            if (flag & ASchedulerIteration::DONT_BLOCK_TIMED) {
                return false;
            }

            auto t = mTasks.front().executionTime;
            mCV.wait_until(lock, t);
            break;
        }
        auto callback = std::move(mTasks.front().callback);
        mTasks.pop_front();
        callback();
    }

    return true;
}


void AScheduler::notifyProcessMessages() {
    mCV.notify_all();
}

void AScheduler::loop() {
    for (;;) {
        iteration();
    }
}

void AScheduler::removeTimer(TimerHandle t) {
    std::unique_lock lock(mSync);

    mTasks.erase(std::remove_if(mTasks.begin(), mTasks.end(), [&](const Task& rhs) {
        return rhs.timer == &*t;
    }), mTasks.end());
    mTimers.erase(t);
    mCV.notify_all();
}
