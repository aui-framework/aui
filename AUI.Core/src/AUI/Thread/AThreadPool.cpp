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

#include "AThreadPool.h"
#include <glm/glm.hpp>
#include <AUI/Common/AException.h>
#include <AUI/Logging/ALogger.h>

AThreadPool::Worker::Worker(AThreadPool& tp, size_t index) :
	mThread(_new<AThread>([&, index]()
{
    mThread->setThreadName("AThreadPool worker #" + AString::number(index + 1));
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
		catch (const AException& e) {
            ALogger::err("uncaught exception in thread pool: " + e.getMessage());
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
    // deadlock fix for mingw
	static AThreadPool* t = new AThreadPool;
	return *t;
}

AThreadPool::AThreadPool(size_t size) {
	for (size_t i = 0; i < size; ++i)
		mWorkers.push_back(new Worker(*this, i));
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

