//
// Created by Alex2772 on 7/16/2022.
//

#include "ACleanup.h"

ACleanup& ACleanup::inst() {
    static ACleanup c;
    return c;
}

void ACleanup::afterEntryPerform() {
    std::unique_lock lock(mSync);

    while (!mCallbacks.empty()) {
        auto callback = std::move(mCallbacks.front());
        mCallbacks.pop();

        lock.unlock(); // avoid deadlock
        callback();
        lock.lock();
    }
}

