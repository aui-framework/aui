/**
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
#include <AUI/Util/kAUI.h>
#include <glm/glm.hpp>

template<typename T>
class AFuture;

namespace aui::detail {
    template<typename T>
    struct future_helper
    {
        using return_type = AFuture<T>;
    };

    template<>
    struct future_helper<void>
    {
        using return_type = void;
    };

}

class API_AUI_CORE AThreadPool
{
private:
    class Worker {
    private:
        bool mEnabled = true;
        _<AThread> mThread;
        std::mutex mMutex;
        bool processQueue(AQueue<std::function<void()>>& queue);
        void thread_fn();
        AThreadPool& mTP;
    public:
        Worker(AThreadPool& tp, size_t index);
        ~Worker();
        void disable();
    };

    struct FenceHelper {
        unsigned enqueuedTasks = 0;
        std::atomic_uint doneTasks;
        bool waitingForNotify = false;
        std::condition_variable cv;
        std::mutex mutex;
    };

    FenceHelper*& fenceHelperStorage();

public:
    enum Priority
    {
        PRIORITY_HIGHEST,
        PRIORITY_MEDIUM,
        PRIORITY_LOWEST,
    };
protected:
    typedef std::function<void()> task;
    AVector<Worker*> mWorkers;
    AQueue<task> mQueueHighest;
    AQueue<task> mQueueMedium;
    AQueue<task> mQueueLowest;
    AQueue<task> mQueueTryLater;
    std::recursive_mutex mQueueLock;
    std::condition_variable mCV;
    std::atomic_uint mIdleWorkers;

public:
    AThreadPool(size_t size);
    AThreadPool();
    ~AThreadPool();
    size_t getPendingTaskCount();
    void run(const std::function<void()>& fun, Priority priority = PRIORITY_MEDIUM);
    void clear();
    void runLaterTasks();
    static void enqueue(const std::function<void()>& fun, Priority priority = PRIORITY_MEDIUM);

    static AThreadPool& global();

    size_t getTotalWorkerCount() const {
        return mWorkers.size();
    }
    size_t getIdleWorkerCount() const {
        return mIdleWorkers;
    }

    /**
     * \brief Wraps lambda. Guarantees that all tasks enqueued inside this lambda will be finished after this function
     *        call.
     * \tparam Callable
     * \param callable
     */
    template<typename Callable>
    void fence(Callable callable) {
        assert(fenceHelperStorage() == nullptr);
        FenceHelper fenceHelper;
        fenceHelperStorage() = &fenceHelper;
        struct s {
            AThreadPool& th;
            s(AThreadPool& th) : th(th) {}

            ~s() {
                th.fenceHelperStorage() = nullptr;
            }
        } s(*this);

        std::unique_lock l(fenceHelper.mutex);
        callable();
        if (fenceHelper.enqueuedTasks == 0)
            return;
        fenceHelper.waitingForNotify = true;
        do {
            fenceHelper.cv.wait(l);
        } while (fenceHelper.doneTasks != fenceHelper.enqueuedTasks);
    }


    /**
     * Parallels work of some range, grouping tasks per thread (i.e. for 8 items on a 4-core processor each core will
     * process 2 items)
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
    typename aui::detail::future_helper<std::invoke_result_t<Callable>>::return_type operator<<(Callable fun)
    {
        if constexpr (std::is_void_v<std::invoke_result_t<Callable>>) {
            run(fun);
        } else
        {
            return AFuture<std::invoke_result_t<Callable>>::make(*this, fun);
        }
    }
    template <typename Callable>
    inline auto operator*(Callable fun)
    {
        return *this << fun;
    }

    class TryLaterException {};
};

#include <AUI/Thread/AFuture.h>

template<typename T>
class AFutureSet: public AVector<AFuture<T>> {
public:
    using AVector<AFuture<T>>::AVector;

    void waitForAll() const {
        for (const AFuture<T>& v : *this) {
            v.operator*();
        }
    }
};

template<typename Iterator, typename Functor>
auto AThreadPool::parallel(Iterator begin, Iterator end, Functor &&functor) {
    using ResultType = decltype(std::declval<Functor>()(std::declval<Iterator>(), std::declval<Iterator>()));
    static_assert(!std::is_same_v<ResultType, void>, "functor must return a value");
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

template <typename Value>
template <typename Callable>
inline AFuture<Value> AFuture<Value>::make(AThreadPool& tp, Callable&& func) noexcept
{
    AFuture<Value> future;
    tp.run([innerWeak = future.mInner.weak(), func = std::forward<Callable>(func)]()
           {
               if (auto inner = innerWeak.lock()) {
                   if (inner->setThread(AThread::current())) return;
                   try {
                       inner = nullptr;
                       auto result = func();
                       nullsafe(innerWeak.lock())->result(std::move(result));
                   } catch (const AException& e) {
                       nullsafe(innerWeak.lock())->reportException(e);
                   }
               }
           }, AThreadPool::PRIORITY_LOWEST);
    return future;
}
