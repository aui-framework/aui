/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once
#include <condition_variable>
#include "AThread.h"

/**
 * @brief Represents a condition variable.
 * @ingroup core
 * @details
 * AConditionVariable extends std::condition_variable with @ref AThread::interrupt() "thread interruption functionality".
 */
class AConditionVariable
{
private:
    std::condition_variable_any mImpl;

    struct WaitHelper {
        WaitHelper(AConditionVariable& var) noexcept {
            auto thread = AThread::current();
            std::unique_lock lock(thread->mCurrentCV.mutex);
            thread->mCurrentCV.cv = &var;
        }
        ~WaitHelper() noexcept(false) {
            auto thread = AThread::current();
            std::unique_lock lock(thread->mCurrentCV.mutex);
            thread->mCurrentCV.cv = nullptr;
        }
    };

    template<typename Predicate>
    struct PredicateHelper {
        Predicate predicate;
        _<AAbstractThread> thread;

        PredicateHelper(Predicate predicate) : predicate(std::move(predicate)), thread(AThread::current()) {}
        bool operator()() noexcept {
            if (thread->isInterrupted()) return true;
            return predicate();
        }
    };

public:
    /**
     * Notifies all observing threads.
     * <dl>
     *   <dt><b>Performance note</b></dt>
     *   <dd>This function is faster than <a href="notify_one">notify_one</a>.</dd>
     * </dl>
     */
    void notify_all() noexcept { mImpl.notify_all(); }

    /**
     * Notifies one observing thread.
     * <dl>
     *   <dt><b>Performance note</b></dt>
     *   <dd>This function is slower than <a href="notify_all">notify_all</a>.</dd>
     * </dl>
     */
    void notify_one() noexcept { mImpl.notify_one(); }

    template<typename Lock>
    void wait(Lock& lock) {
        {
            WaitHelper w(*this);
            mImpl.wait(lock);
        }
        AThread::interruptionPoint();
    }

    /**
     * Waits for the notification.
     * @param lock lock.
     * @param predicate which returns false if the waiting should be continued.
     */
    template<typename Lock, typename Predicate>
    void wait(Lock& lock, Predicate&& predicate) {
        {
            WaitHelper w(*this);
            mImpl.wait(lock, PredicateHelper(std::forward<Predicate>(predicate)));
        }
        AThread::interruptionPoint();
    }

    /**
     * Waits for the notification.
     * @param lock lock.
     * @param duration duration to waitForExitCode for.
     */
    template<typename Lock, typename Duration>
    void wait_for(Lock& lock, Duration duration) {
        {
            WaitHelper w(*this);
            mImpl.wait_for(lock, duration);
        }
        AThread::interruptionPoint();
    }

    /**
     * Waits for the notification.
     * @param lock lock.
     * @param duration duration to waitForExitCode for.
     * @param predicate which returns false if the waiting should be continued.
     */
    template<typename Lock, typename Duration, typename Predicate>
    void wait_for(Lock& lock, Duration duration, Predicate&& predicate) {
        {
            WaitHelper w(*this);
            mImpl.wait_for(lock, duration, PredicateHelper(std::forward<Predicate>(predicate)));
        }
        AThread::interruptionPoint();
    }
};
