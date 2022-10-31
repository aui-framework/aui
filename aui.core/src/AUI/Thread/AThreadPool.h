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

#include <AUI/Core.h>
#include <cassert>
#include <atomic>

#include <AUI/Common/AVector.h>
#include <AUI/Common/AQueue.h>
#include <AUI/Common/AException.h>
#include <AUI/Thread/AThread.h>
#include <AUI/Util/kAUI.h>
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

    template <typename Callable>
    [[nodiscard]]
    inline auto operator*(Callable fun)
    {
        using Value = std::invoke_result_t<Callable>;
        AFuture<Value> future(std::move(fun));
        run([innerWeak = future.inner().weak()]()
            {
                if (auto lock = innerWeak.lock()) {
                    auto innerUnsafePointer = lock->ptr().get(); // using .get() here in order to bypass
                                                                 // null check in operator->
                    lock = nullptr;
                    innerUnsafePointer->tryExecute(innerWeak);
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
