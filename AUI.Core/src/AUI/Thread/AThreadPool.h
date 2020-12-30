#pragma once

#include <AUI/Core.h>

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
     * \brief Оборачивает лямбду, гарантируя, что все задачи, которые были отправлены на выполнение в этот тредпул
     *        внутри этой лямбды будут завершены после вызова этой функции.
     * @tparam Callable
     * @param callable
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