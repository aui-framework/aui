/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Core.h>
#include <cassert>
#include <atomic>

#include "AUI/Common/AVector.h"
#include "AUI/Common/Queue.h"

template<typename T>
class AFuture;

namespace aui::detail {
    template<typename T>
    struct future_helper
    {
        typedef _<AFuture<T>> return_type;
    };

    template<>
    struct future_helper<void>
    {
        typedef void return_type;
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
		bool processQueue(Queue<std::function<void()>>& queue);
		void thread_fn();
		AThreadPool& mTP;
	public:
		Worker(AThreadPool& tp);
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
	Queue<task> mQueueHighest;
	Queue<task> mQueueMedium;
	Queue<task> mQueueLowest;
	Queue<task> mQueueTryLater;
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