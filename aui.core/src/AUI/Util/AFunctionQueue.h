#pragma once

#include <AUI/Thread/AMutex.h>
#include <AUI/Common/AQueue.h>

/**
 * @brief Thread-safe implementation of function queue.
 * @ingroup core
 */
class AFunctionQueue {
public:


    void operator<<(std::function<void()> function) {
        std::unique_lock lock(mLock);
        mQueue << std::move(function);
    }

    void process() {
        std::unique_lock l(mLock);
        while (!mQueue.empty()) {
            auto f = std::move(mQueue.front());
            mQueue.pop();
            l.unlock();
            f();
            l.lock();
        }
    }

    [[nodiscard]]
    bool empty() noexcept {
        std::unique_lock l(mLock);
        return mQueue.empty();
    }

private:
    AMutex mLock;
    AQueue<std::function<void()>> mQueue;
};