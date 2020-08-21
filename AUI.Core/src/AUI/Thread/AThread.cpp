#include "AThread.h"

#include <cassert>
#include <AUI/Common/AException.h>
#include <AUI/Logging/ALogger.h>

#include "IEventLoop.h"

AAbstractThread::AAbstractThread(const id& id): mId(id)
{
}

_<AAbstractThread>& AAbstractThread::threadStorage()
{
	thread_local _<AAbstractThread> thread;
	return thread;
}

AAbstractThread::~AAbstractThread()
= default;

bool AAbstractThread::isInterrupted()
{
	// для абстрактного потока - это всегда false.
	return false;
}

void AAbstractThread::resetInterrupted()
{
}

void AThread::start()
{
	assert(mThread == nullptr);
	auto t = shared_from_this();
	mThread = new std::thread([&, t] ()
	{
		threadStorage() = t;
		auto f = mFunctor;
		mFunctor = nullptr;
		mId = std::this_thread::get_id();
		
		try {
			f();
		} catch (const AException& e) {
            ALogger::err("uncaught exception: " + e.getMessage());
            abort();
        } catch (AInterrupted)
		{
			
		}
	});
}

void AThread::interrupt()
{
	mInterrupted = true;
	mSleepCV.notify_one();
}

void AThread::sleep(unsigned durationInMs)
{
	std::mutex m;
	std::unique_lock lock(m);
	current()->mSleepCV.wait_for(lock, std::chrono::milliseconds(durationInMs));
	interruptionPoint();
}

AAbstractThread::id AAbstractThread::getId() const
{
	return mId;
}

_<AAbstractThread> AThread::current()
{
	auto& t = threadStorage();
	if (t == nullptr) // абстрактный поток
	{
		t = _<AAbstractThread>(new AAbstractThread(std::this_thread::get_id()));
	}

	return t;
}

void AThread::interruptionPoint()
{
	if (current()->isInterrupted())
	{
		current()->resetInterrupted();
		throw AInterrupted();
	}
}

bool AThread::isInterrupted()
{
	return mInterrupted;
}

void AThread::resetInterrupted()
{
	mInterrupted = false;
}

void AThread::join()
{
	mThread->join();
}

void AAbstractThread::enqueue(const std::function<void()>& f)
{
	{
		std::unique_lock lock(mQueueLock);
		mMessageQueue << f;
	}
	{
		std::unique_lock lock(mEventLoopLock);
		if (mCurrentEventLoop)
		{
			mCurrentEventLoop->notifyProcessMessages();
		}
	}
}

void AAbstractThread::processMessages()
{
	std::unique_lock lock(mQueueLock);
	while (!mMessageQueue.empty())
	{
		auto f = mMessageQueue.front();
		mMessageQueue.pop_front();
		lock.unlock();
		f();
		lock.lock();
	}
}

AThread::~AThread()
{
	if (mThread) {
		interrupt();
		if (mThread->get_id() == std::this_thread::get_id()) {
			mThread->detach();
			delete mThread;
		}
		else {
			if (mThread->joinable())
				mThread->join();
			delete mThread;
		}
		mThread = nullptr;
	}
}
