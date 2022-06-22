#pragma once

#include "AMutex.h"
#include "AConditionVariable.h"

/**
 * @brief One-shot event communication.
 * @ingroup core
 * @details
 * Used for guaranteed signal from slave thread to master thread (i.e. wait till slave thread starts).
 */
class ACutoffSignal {
public:
    ACutoffSignal() = default;

    /**
     * @brief Flags a signal, causing the waitForSignal() to never block.
     */
    void makeSignal() {
        std::unique_lock lock(mMutex);
        mSignaled = true;
        mCV.notify_all();
    }

    /**
     * @brief Resets the cut off state so the waitForSignal() function would block again.
     */
     void resetSignal() noexcept {
        mSignaled = false;
    }

    /**
     * @brief Wait for makeSignal().
     * @details
     * waitForSignal() blocks the thread until makeSignal() is not called by other thread.
     *
     * Unlike condition variable, the second call to makeSignal() will not block. To override this behaviour, use the
     * resetSignal() function.
     */
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


