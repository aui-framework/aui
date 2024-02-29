// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <condition_variable>
#include "AThread.h"
#include "AUI/Performance/APerformanceSection.h"

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
        APerformanceSection section("Conditional Variable");
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
        APerformanceSection section("Conditional Variable");
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
        APerformanceSection section("Conditional Variable");
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
        APerformanceSection section("Conditional Variable");
        {
            WaitHelper w(*this);
            mImpl.wait_for(lock, duration, PredicateHelper(std::forward<Predicate>(predicate)));
        }
        AThread::interruptionPoint();
    }

    /**
     * Waits until the notification.
     * @param lock lock.
     * @param duration duration to waitForExitCode until.
     */
    template<typename Lock, typename Timepoint>
    void wait_until(Lock& lock, Timepoint timepoint) {
        APerformanceSection section("Conditional Variable");
        {
            WaitHelper w(*this);
            mImpl.wait_until(lock, timepoint);
        }
        AThread::interruptionPoint();
    }

    /**
     * Waits until the notification.
     * @param lock lock.
     * @param duration duration to waitForExitCode until.
     * @param predicate which returns false if the waiting should be continued.
     */
    template<typename Lock, typename Duration, typename Predicate>
    void wait_until(Lock& lock, Duration duration, Predicate&& predicate) {
        APerformanceSection section("Conditional Variable");
        {
            WaitHelper w(*this);
            mImpl.wait_until(lock, duration, PredicateHelper(std::forward<Predicate>(predicate)));
        }
        AThread::interruptionPoint();
    }
};
