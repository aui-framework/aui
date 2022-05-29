#pragma once

#include "AMutex.h"
#include "AConditionVariable.h"

/**
 * Used for guaranteed signal from slave thread to master thread (i.e. wait till slave thread starts).
 */
class ACutoffSignal {
public:
    ACutoffSignal() noexcept = default;

    void makeSignal() {
        std::unique_lock lock(mMutex);
        mSignaled = true;
        mCV.notify_all();
    }

    void waitForSignal() {
        if (!mSignaled) {
            std::unique_lock lock(mMutex);
            mCV.wait(lock, [this] {
                return mSignaled;
            });
        }
    }

private:
    AMutex mMutex;
    AConditionVariable mCV;
    bool mSignaled = false;
};


