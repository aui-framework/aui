#include "AThreadPool.h"
#include <glm/glm.hpp>

AThreadPool::Worker::Worker(AThreadPool& tp) :
	mThread(_new<AThread>([&]()
{
	thread_fn();
})),
mTP(tp)
{
	mThread->start();
}

bool AThreadPool::Worker::processQueue(Queue<std::function<void()>>& queue)
{
	std::unique_lock tpLock(mTP.mQueueLock);
	if (!queue.empty()) {
		auto func = std::move(queue.front());
		queue.pop();
		tpLock.unlock();
		try {
			func();
            if (auto h = mTP.fenceHelperStorage()) {
                if ((h->doneTasks += 1) == h->enqueuedTasks) {
                    if (h->waitingForNotify) {
                        std::unique_lock lock(h->mutex);
                        if (h->waitingForNotify) {
                            h->cv.notify_one();
                        }
                    }
                }
            }
		}
		catch (const AThread::AInterrupted&)
		{
			//AThread::current()->resetInterruptFlag();
		}
		catch (const TryLaterException&)
		{
			tpLock.lock();
			mTP.mQueueTryLater.push(func);
			tpLock.unlock();
		}
		return true;
	}
	return false;
}

void AThreadPool::Worker::thread_fn() {
	while (mEnabled) {
		while (!mTP.mQueueHighest.empty() || !mTP.mQueueMedium.empty() || !mTP.mQueueLowest.empty()) {
			if (processQueue(mTP.mQueueHighest))
				continue;
			if (processQueue(mTP.mQueueMedium))
				continue;
			processQueue(mTP.mQueueLowest);
		}
		std::unique_lock<std::mutex> lck(mMutex);
        mTP.mIdleWorkers += 1;
		mTP.mCV.wait(lck);
        mTP.mIdleWorkers -= 1;
	}
}

AThreadPool::Worker::~Worker() {
	mEnabled = false;
	mThread->interrupt();
	mThread->join();
	mThread = nullptr;
}

void AThreadPool::Worker::disable() {
	mEnabled = false;
}


void AThreadPool::run(const std::function<void()>& fun, Priority priority) {
	std::unique_lock lck(mQueueLock);
	if (auto h = fenceHelperStorage()) {
	    h->enqueuedTasks += 1;
	}
	switch (priority)
	{
	case PRIORITY_MEDIUM:
		mQueueMedium.push(fun);
		break;
	case PRIORITY_HIGHEST:
		mQueueHighest.push(fun);
		break;
	case PRIORITY_LOWEST:
		mQueueLowest.push(fun);
		break;
	}
	if (mIdleWorkers > 0) {
        mCV.notify_one();
    }
}

void AThreadPool::clear()
{
	std::unique_lock lck(mQueueLock);

	while (!mQueueLowest.empty())
		mQueueLowest.pop();
	while (!mQueueMedium.empty())
		mQueueMedium.pop();
	while (!mQueueHighest.empty())
		mQueueHighest.pop();
	while (!mQueueTryLater.empty())
		mQueueTryLater.pop();

}

void AThreadPool::runLaterTasks()
{
	std::unique_lock lck(mQueueLock);
	while (!mQueueTryLater.empty())
	{
		mQueueLowest.emplace(std::move(mQueueTryLater.front()));
		mQueueTryLater.pop();
	}
	mCV.notify_one();
}

void AThreadPool::enqueue(const std::function<void()>& fun, Priority priority)
{
	global().run(fun, priority);
}

AThreadPool& AThreadPool::global()
{
    // говнофикс дедлока на mingw
	static AThreadPool* t = new AThreadPool;
	return *t;
}

AThreadPool::AThreadPool(size_t size) {
	for (size_t i = 0; i < size; ++i)
		mWorkers.push_back(new Worker(*this));
}

AThreadPool::AThreadPool() :
	AThreadPool(glm::max(std::thread::hardware_concurrency() - 1, 1u))
{
}

AThreadPool::~AThreadPool() {
	for (auto& f : mWorkers) {
		f->disable();
	}
	for (auto& f : mWorkers) {
		mCV.notify_all();
		delete f;
	}
}

size_t AThreadPool::getPendingTaskCount()
{
	return mQueueHighest.size() + mQueueLowest.size() + mQueueMedium.size();
}

AThreadPool::FenceHelper*& AThreadPool::fenceHelperStorage() {
    static FenceHelper* helper = nullptr;
    return helper;
}

