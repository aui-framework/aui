// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#include "AThreadPool.h"
#include <glm/glm.hpp>
#include <AUI/Common/AException.h>
#include <AUI/Logging/ALogger.h>

AThreadPool::Worker::Worker(AThreadPool& tp, size_t index) :
	mThread(_new<AThread>([&, index]()
{
    AThread::setName("AThreadPool #" + AString::number(index + 1));
	thread_fn();
})),
mTP(tp)
{
	mThread->start();
}

bool AThreadPool::Worker::processQueue(std::unique_lock<std::mutex>& mutex, AQueue<std::function<void()>>& queue)
{
	if (!queue.empty()) {
		auto func = std::move(queue.front());
		queue.pop();
		mutex.unlock();
		try {
			func();
		}
		catch (const AException& e) {
            ALogger::err("uncaught exception in thread pool: " + e.getMessage());
        }
		catch (const AThread::Interrupted&)
		{
			//AThread::current()->resetInterruptFlag();
		}
		catch (const TryLaterException&)
		{
			mutex.lock();
			mTP.mQueueTryLater.push(func);
			return true;
		}
		mutex.lock();
		return true;
	}
	return false;
}

void AThreadPool::Worker::thread_fn() {
	std::unique_lock tpLock(mTP.mQueueLock);
	while (mEnabled) {
		while (!mTP.mQueueHighest.empty() || !mTP.mQueueMedium.empty() || !mTP.mQueueLowest.empty()) {
			if (processQueue(tpLock, mTP.mQueueHighest))
				continue;
			if (processQueue(tpLock, mTP.mQueueMedium))
				continue;
			processQueue(tpLock, mTP.mQueueLowest);
		}
#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"
        if (!mEnabled) return;
#pragma clang diagnostic pop
        mTP.mIdleWorkers += 1;
		assert(tpLock.owns_lock());
		mTP.mCV.wait(tpLock);
        mTP.mIdleWorkers -= 1;
	}
}

AThreadPool::Worker::~Worker() {
	mThread->join();
	mThread = nullptr;
}

void AThreadPool::Worker::aboutToDelete() {
	mEnabled = false;
}


void AThreadPool::run(const std::function<void()>& fun, Priority priority) {
	std::unique_lock lck(mQueueLock);

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
    mWorkers.reserve(size);
	for (size_t i = 0; i < size; ++i)
		mWorkers.push_back(std::make_unique<Worker>(*this, i));
}

AThreadPool::AThreadPool() :
	AThreadPool(glm::max(std::thread::hardware_concurrency() - 1, 2u))
{
}

AThreadPool::~AThreadPool() {
	std::unique_lock lck(mQueueLock);
	for (auto& f : mWorkers) {
        f->aboutToDelete();
	}
	for (auto& f : mWorkers) {
        mCV.notify_all();
		lck.unlock();
		f.reset();
		lck.lock();
	}
}

void AThreadPool::setWorkersCount(std::size_t workersCount) {
    assert(("invalid worker count", workersCount >= 2 && workersCount <= 1000));
    std::unique_lock lck(mQueueLock);
    if (mWorkers.size() >= workersCount) {
        while (mWorkers.size() > workersCount) {
            mCV.notify_all();
            mWorkers.last()->aboutToDelete();
            lck.unlock();
            mWorkers.pop_back();
            lck.lock();
        }
    } else {
        // have to add new workers
        mWorkers.reserve(workersCount);
        while (mWorkers.size() < workersCount) {
            mWorkers.push_back(std::make_unique<Worker>(*this, mWorkers.size()));
        }
    }
}

size_t AThreadPool::getPendingTaskCount()
{
	return mQueueHighest.size() + mQueueLowest.size() + mQueueMedium.size();
}
