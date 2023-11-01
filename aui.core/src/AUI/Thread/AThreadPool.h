// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include <AUI/Core.h>
#include <cassert>
#include <atomic>

#include <AUI/Common/AVector.h>
#include <AUI/Common/AQueue.h>
#include <AUI/Common/AException.h>
#include <AUI/Thread/AThread.h>
#include <glm/glm.hpp>
#include <utility>

template<typename T>
class AFuture;


/**
 * @brief Thread pool implementation.
 * @see AThreadPool::global()
 */
class API_AUI_CORE AThreadPool
{
private:
    class Worker {
    private:
        bool mEnabled = true;
        _<AThread> mThread;
        bool processQueue(std::unique_lock<std::mutex>& mutex, AQueue<std::function<void()>>& queue);
        void thread_fn();
        AThreadPool& mTP;
    public:
        Worker(AThreadPool& tp, size_t index);
        ~Worker();
        void aboutToDelete();
    };

public:
    enum Priority
    {
        PRIORITY_HIGHEST,
        PRIORITY_MEDIUM,
        PRIORITY_LOWEST,
    };
protected:
    typedef std::function<void()> task;
    AVector<std::unique_ptr<Worker>> mWorkers;
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
     * @brief Initializes the thread pool with <code>max(std::thread::hardware_concurrency() - 1, 2)</code> of threads.
     */
    AThreadPool();
    ~AThreadPool();
    size_t getPendingTaskCount();
    void run(const std::function<void()>& fun, Priority priority = PRIORITY_MEDIUM);
    void clear();
    void runLaterTasks();
    static void enqueue(const std::function<void()>& fun, Priority priority = PRIORITY_MEDIUM);

    void setWorkersCount(std::size_t workersCount);

    /**
     * @return a global thread pool created with the default constructor.
     */
    static AThreadPool& global();

    size_t getTotalWorkerCount() const {
        return mWorkers.size();
    }
    size_t getIdleWorkerCount() const {
        return mIdleWorkers;
    }

    /**
     * Parallels work of some range, grouping tasks per thread (i.e. for 8 items on a 4-core processor each core will
     * process 2 items)
     *
     * @param begin range begin
     * @param end range end
     * @param functor a functor of the following signature:
     * @code{cpp} Result(Iterator begin, Iterator end) @endcode
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
    template<typename Iterator, typename Functor>
    auto parallel(Iterator begin, Iterator end, Functor&& functor);

    template <aui::invocable Callable>
    [[nodiscard]]
    inline auto operator*(Callable fun)
    {
        using Value = std::invoke_result_t<Callable>;
        AFuture<Value> future(std::move(fun));
        run([innerWeak = future.inner().weak()]()
            {
                /*
                 * Avoid holding a strong reference - we need to keep future cancellation on reference count exceeding
                 * even while actual future execution.
                 */
                if (auto lock = innerWeak.lock()) {
                    auto innerUnsafePointer = lock->ptr().get(); // using .get() here in order to bypass
                                                                 // null check in operator->

                    lock = nullptr;                              // destroy strong ref

                    innerUnsafePointer->tryExecute(innerWeak);   // there's a check inside tryExecute to check its validity
                }
            }, AThreadPool::PRIORITY_LOWEST);
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
 * AFutureSet involves functions to efficiently manage multiple AFutures.
 *
 * Guarantees that held futures will never be executed or be during execution after AAsyncHolder is destroyed.
 */
template<typename T = void>
class AFutureSet: public AVector<AFuture<T>> {
private:
    using super = AVector<AFuture<T>>;
public:
    using AVector<AFuture<T>>::AVector;

    /**
     * @brief Wait for the result of every AFuture.
     * @deprecated use onAllComplete instead.
     */
    void waitForAll() const {
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
                v.operator*(); // TODO bad design
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
     * @note AFutureSet is not required to be alive when AFutures would potentially call onSuccess callback since a
     * temporary object is created to keep track of the task completeness.
     */
    template<aui::invocable OnComplete>
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

template<typename Iterator, typename Functor>
auto AThreadPool::parallel(Iterator begin, Iterator end, Functor&& functor) {
    using ResultType = decltype(std::declval<Functor>()(std::declval<Iterator>(), std::declval<Iterator>()));
    AFutureSet<ResultType> futureSet;

    size_t itemCount = end - begin;
    size_t affinity = (glm::min)(AThreadPool::global().getTotalWorkerCount(), itemCount);
    if (affinity == 0) return futureSet;
    size_t itemsPerThread = itemCount / affinity;

    for (size_t threadIndex = 0; threadIndex < affinity; ++threadIndex) {
        auto forThreadBegin = begin;
        begin += itemsPerThread;
        auto forThreadEnd = threadIndex + 1 == affinity ? end : begin;
        futureSet.push_back(*this * [functor = std::forward<Functor>(functor), forThreadBegin, forThreadEnd]() -> decltype(auto) {
            return functor(forThreadBegin, forThreadEnd);
        });
    }

    return futureSet;
}


#include <AUI/Reflect/AReflect.h>
