#pragma once

#include "AFuture.h"

/**
 * @brief Holds set of futures keeping them valid. Guarantees that held futures will never be executed or be during
 * execution after AAsyncHolder is destroyed.
 */
class AAsyncHolder {
public:
    AAsyncHolder() = default;
    ~AAsyncHolder() {
        std::unique_lock lock(mSync);

        while (!mFutureSet.empty()) {
            auto l = std::move(mFutureSet.last());
            mFutureSet.pop_back();
            lock.unlock();
            *l;
            lock.lock();
        }
    }

    AAsyncHolder& operator<<(AFuture<> future) {
        std::unique_lock lock(mSync);
        future.onSuccess([this, future]() {
            std::unique_lock lock(mSync);
            mFutureSet.removeFirst(future);
            const_cast<AFuture<>&>(future) = nullptr;
        });
        mFutureSet << std::move(future);
        return *this;
    }

    [[nodiscard]]
    std::size_t size() const {
        std::unique_lock lock(mSync);
        return mFutureSet.size();
    }

private:
    mutable AMutex mSync;
    AFutureSet<> mFutureSet;
};