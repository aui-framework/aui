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

#include <chrono>
#include <functional>
#include <list>
#include "AUI/Reflect/AEnumerate.h"
#include <AUI/Thread/AMutex.h>
#include <AUI/Thread/AConditionVariable.h>
#include <AUI/Thread/IEventLoop.h>
#include <AUI/Util/ABitField.h>


/**
 * @brief Used for AScheduler::iteration().
 */
AUI_ENUM_FLAG(ASchedulerIteration) {
    NONE = 0,

    /**
     * @brief Disables infinite block.
     * @details
     * By default, scheduler blocks the thread infinitely until some task or timer added or
     * AScheduler::notifyProcessMessages() is called if there are no pending tasks. This flag disables this behaviour
     * and causes AScheduler::iteration() to return false immediately in such case.
     */
    DONT_BLOCK_INFINITELY = 0b1,

    /**
     * @brief Disables timed block.
     * @details
     * By default, scheduler may block the thread until some task or timer can be executed or
     * AScheduler::notifyProcessMessages() is called. This flag disables this behaviour and causes
     * AScheduler::iteration() to return false immediately in such case.
     */
    DONT_BLOCK_TIMED = 0b10,

    /**
     * @brief A combination of DONT_BLOCK_INFINITELY and DONT_BLOCK_TIMED.
     */
    DONT_BLOCK = DONT_BLOCK_INFINITELY | DONT_BLOCK_TIMED,
};

/**
 * @brief Basic scheduler used for timers.
 * @ingroup core
 */
class API_AUI_CORE AScheduler: public IEventLoop {
private:
    using SchedulerDuration = std::chrono::microseconds;

    struct Timer {
        std::chrono::milliseconds timeout;
        std::chrono::high_resolution_clock::time_point nextExecution;
        std::function<void()> callback;
    };

    struct Task {
        std::chrono::high_resolution_clock::time_point executionTime;
        std::function<void()> callback;
        _weak<Timer> timer;
    };
public:
    using TimerHandle = _weak<Timer>;

    AScheduler();

    /**
     * @brief Performs an iteration.
     * @param flag flags for the iteration. See ASchedulerIteration for more info.
     * @return true, if some action taken, false otherwise
     */
    bool iteration(ABitField<ASchedulerIteration> flag = ASchedulerIteration::NONE);

    void notifyProcessMessages() override;
    void loop() override;

    template<typename Duration>
    void enqueue(Duration timeout, std::function<void()> callback) {
        std::unique_lock lock(mSync);
        Task asTask = {
                std::chrono::duration_cast<SchedulerDuration>(timeout) + currentTime(),
                std::move(callback)
        };
        enqueueTask(std::move(asTask));
        mCV.notify_all();
    }

    /**
     * @brief Creates a timer.
     * @param timeout timeout (i.e. 500ms)
     * @param callback callback to be called
     * @return timer instance which can be used to remove the timer.
     * @details
     * Creates a timer with the specified callback. The callback is not called immediately during timer creation.
     */
    template<typename Duration>
    TimerHandle timer(Duration timeout, std::function<void()> callback) {
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(timeout);
        Timer t = {
                millis,
                millis + currentTime(),
                std::move(callback)
        };
        auto asTimer = _new<Timer>(std::move(t));

        std::unique_lock lock(mSync);
        mTimers.push_back(asTimer);
        enqueueTimer(asTimer);
        return asTimer;
    }


    void removeTimer(const TimerHandle& t);

    [[nodiscard]]
    bool emptyTasks() const noexcept {
        return mTasks.empty();
    }

    void stop() {
        mIsRunning = false;
        mCV.notify_all();
    }


private:
    AMutex mSync;
    AConditionVariable mCV;
    bool mIsRunning = false;

    std::list<Task> mTasks;
    std::list<_<Timer>> mTimers;

    static std::chrono::high_resolution_clock::time_point currentTime() noexcept {
        return std::chrono::high_resolution_clock::now();
    }

    void enqueueTimer(const _<Timer>& timer) {
        Task t = {
                timer->nextExecution,
                [this, timer]() {
                    timer->callback();
                    std::unique_lock lock(mSync);
                    enqueueTimer(timer);
                },
                timer
        };
        timer->nextExecution += timer->timeout;
        enqueueTask(std::move(t));
    }


    void enqueueTask(Task&& asTask) {
        auto whereToInsert = std::find_if(mTasks.begin(), mTasks.end(), [&](const Task& rhs) {
            return asTask.executionTime < rhs.executionTime;
        });

        mTasks.insert(whereToInsert, std::move(asTask));
        mCV.notify_all();
    }

};
