/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Core.h>
#include <cassert>
#include <atomic>

#include <AUI/Common/AVector.h>
#include <AUI/Common/AQueue.h>
#include <AUI/Common/AException.h>
#include <AUI/Thread/AThread.h>
#include <glm/glm.hpp>
#include <utility>
#include "AUI/Traits/concepts.h"

template <typename T>
class AFuture;

/**
 * @brief Thread pool implementation.
 * @see AThreadPool::global()
 */
class API_AUI_CORE AThreadPool {
public:
    class API_AUI_CORE Worker : public AThread {
    private:
        bool mEnabled = true;
        bool processQueue(std::unique_lock<std::mutex>& mutex, AQueue<std::function<void()>>& queue);
        AThreadPool& mTP;

        void iteration(std::unique_lock<std::mutex>& tpLock);
        void wait(std::unique_lock<std::mutex>& tpLock);

    public:
        Worker(AThreadPool& tp, size_t index);
        ~Worker();
        void aboutToDelete();

        template <aui::predicate ShouldContinue>
        void loop(ShouldContinue&& shouldContinue) {
            std::unique_lock lock(mTP.mQueueLock);
            while (shouldContinue()) {
                iteration(lock);
                if (!shouldContinue()) {
                    return;
                }
                wait(lock);
            }
        }

        AThreadPool& threadPool() noexcept { return mTP; }
    };

    enum Priority {
        PRIORITY_HIGHEST,
        PRIORITY_MEDIUM,
        PRIORITY_LOWEST,
    };

protected:
    typedef std::function<void()> task;
    AVector<_<Worker>> mWorkers;
    AQueue<task> mQueueHighest;
    AQueue<task> mQueueMedium;
    AQueue<task> mQueueLowest;
    AQueue<task> mQueueTryLater;
    std::mutex mQueueLock;
    std::condition_variable mCV;
    size_t mIdleWorkers = 0;

public:
    /**
     * @brief Initializes the thread pool with size of threads.
     * @param size thread count to initialize.
     */
    AThreadPool(size_t size);

    /**
     * @brief Initializes the thread pool with <code>max(std::thread::hardware_concurrency() - 1, 2)</code> of threads
     * or --aui-threadpool-size=SIZE passed to your application.
     */
    AThreadPool();
    ~AThreadPool();
    size_t getPendingTaskCount();
    size_t getTotalTaskCount() {
        return getPendingTaskCount() + getTotalWorkerCount() - getIdleWorkerCount();
    }
    void run(const std::function<void()>& fun, Priority priority = PRIORITY_MEDIUM);
    void clear();
    void runLaterTasks();
    static void enqueue(const std::function<void()>& fun, Priority priority = PRIORITY_MEDIUM);

    void setWorkersCount(std::size_t workersCount);

    void wakeUpAll() {
        std::unique_lock lck(mQueueLock);
        mCV.notify_all();
    }

    /**
     * @brief Global thread pool created with the default constructor.
     */
    static AThreadPool& global();

    [[nodiscard]]
    const AVector<_<Worker>>& workers() const {
        return mWorkers;
    }

    size_t getTotalWorkerCount() const { return mWorkers.size(); }
    size_t getIdleWorkerCount() const { return mIdleWorkers; }

    /**
     * Parallels work of some range, grouping tasks per thread (i.e. for 8 items on a 4-core processor each core will
     * process 2 items)
     *
     * @param begin range begin
     * @param end range end
     * @param functor a functor of the following signature:
     * ```cpp Result(Iterator begin, Iterator end) ```
     *
     * @return future set per thread (i.e. for 8 items on a 4-core processor there will be 4 futures)
     *
     * <dl>
     *   <dt><b>Performance note</b></dt>
     *   <dd>
     *      When this function is used to write to the source data it would not be L1-cache friendly. Consider writing
     *      results to another location.
     *   </dd>
     * </dl>
     */
    template <typename Iterator, typename Functor>
    auto parallel(Iterator begin, Iterator end, Functor&& functor);

    template <aui::invocable Callable>
    [[nodiscard]] inline auto operator*(Callable fun) {
        using Value = std::invoke_result_t<Callable>;
        AFuture<Value> future(std::move(fun));
        run(
            [innerWeak = future.inner().weak()]() {
                /*
                 * Avoid holding a strong reference - we need to keep future cancellation on reference count exceeding
                 * even while actual future execution.
                 */
                if (auto lock = innerWeak.lock()) {
                    auto innerUnsafePointer = lock->ptr().get();   // using .get() here in order to bypass
                                                                   // null check in operator->

                    lock = nullptr;   // destroy strong ref

                    innerUnsafePointer->tryExecute(innerWeak);   // there's a check inside tryExecute to check its
                                                                 // validity
                }
            },
            AThreadPool::PRIORITY_LOWEST);
        return future;
    }

    class TryLaterException {};
};

#include <AUI/Thread/AFuture.h>

/**
 * @brief Manages multiple futures.
 * @ingroup core
 * @tparam T future's return type
 * @details
 * <!-- aui:experimental -->
 * AFutureSet involves functions to efficiently manage multiple AFutures.
 *
 * Guarantees that held futures will never be executed or be during execution after AAsyncHolder is destroyed.
 */
template <typename T = void>
class AFutureSet : public AVector<AFuture<T>> {
private:
    using super = AVector<AFuture<T>>;

public:
    using AVector<AFuture<T>>::AVector;

    /**
     * @brief Wait for the result of every AFuture.
     * @deprecated use onAllComplete instead.
     */
    void waitForAll() {
        // wait from the end to avoid idling (see AFuture::wait for details)
        for (const AFuture<T>& v : aui::reverse_iterator_wrap(*this)) {
            v.operator*();
        }
    }

    /**
     * @brief Find AFutures that encountered an exception. If such AFuture is found, AInvocationTargetException is
     * thrown.
     */
    void checkForExceptions() const {
        for (const AFuture<T>& v : *this) {
            if (v.hasResult()) {
                v.operator*();   // TODO bad design
            }
        }
    }

    /**
     * @brief Specifies a callback which will be called when all futures in future set would have the result.
     * @details
     * Even if all tasks are already completed, it's guaranteed that your callback will be called.
     *
     * The thread on which your callback will be called is undefined.
     *
     * AFutureSet is not required to be alive when AFutures would potentially call onSuccess callback since a
     * temporary object is created to keep track of the task completeness.
     */
    template <aui::invocable OnComplete>
    void onAllComplete(OnComplete&& onComplete) {
        // check if all futures is already complete.
        for (const AFuture<T>& v : *this) {
            if (!v.hasResult()) {
                goto setupTheHell;
            }
        }
        onComplete();
        return;

    setupTheHell:
        struct Temporary {
            OnComplete onComplete;
            AFutureSet myCopy;
            std::atomic_bool canBeCalled = true;
        };
        auto temporary = _new<Temporary>(std::forward<OnComplete>(onComplete), *this);
        for (const AFuture<T>& v : *this) {
            v.onSuccess([temporary](const auto& v) {
                for (const AFuture<T>& v : temporary->myCopy) {
                    if (!v.hasResult()) {
                        return;
                    }
                }
                // yay! all tasks are completed. the last thing to check if the callback is already called
                if (temporary->canBeCalled.exchange(false)) {
                    temporary->onComplete();
                }
            });
        }
    }
};

template <typename Iterator, typename Functor>
auto AThreadPool::parallel(Iterator begin, Iterator end, Functor&& functor) {
    using ResultType = decltype(std::declval<Functor>()(std::declval<Iterator>(), std::declval<Iterator>()));
    AFutureSet<ResultType> futureSet;

    size_t itemCount = end - begin;
    size_t affinity = (glm::min) (AThreadPool::global().getTotalWorkerCount(), itemCount);
    if (affinity == 0)
        return futureSet;
    size_t itemsPerThread = itemCount / affinity;

    for (size_t threadIndex = 0; threadIndex < affinity; ++threadIndex) {
        auto forThreadBegin = begin;
        begin += itemsPerThread;
        auto forThreadEnd = threadIndex + 1 == affinity ? end : begin;
        futureSet.push_back(
            *this * [functor = std::forward<Functor>(functor), forThreadBegin, forThreadEnd]() -> decltype(auto) {
                return functor(forThreadBegin, forThreadEnd);
            });
    }

    return futureSet;
}

#include <AUI/Reflect/AReflect.h>
