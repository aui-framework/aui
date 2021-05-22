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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#include "AThread.h"

#include <cassert>
#include <AUI/Common/AException.h>
#include <AUI/Common/AString.h>
#include <AUI/Logging/ALogger.h>

#include "IEventLoop.h"

#ifdef _WIN32
#include <Windows.h>


void setThreadNameImpl(HANDLE handle, const AString& name) {
    static struct SetThreadDescription {
    private:
        using T = HRESULT(WINAPI*)(HANDLE, PCWSTR);
        T mPtr = nullptr;

    public:
        SetThreadDescription() {
            auto hKernel32 = GetModuleHandleA("kernel32.dll");
            if (hKernel32) {
                mPtr = reinterpret_cast<T>(GetProcAddress(hKernel32, "SetThreadDescription"));
            }
        }

        operator bool() const {
            return mPtr != nullptr;
        }

        HRESULT operator()(HANDLE thread, PCWSTR name) {
            return mPtr(thread, name);
        }
    } s;
    if (s) {
        s(handle, name.c_str());
    }
}
#endif

class CurrentThread: public AAbstractThread {
public:
    using AAbstractThread::AAbstractThread;

    void setThreadName(const AString& name) override {
#ifdef _WIN32
        setThreadNameImpl(GetCurrentThread(), name);
#endif
    }
};

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
	// abstract thread could not be interrupted.
	return false;
}

void AAbstractThread::interrupt() {

}

void AAbstractThread::resetInterruptFlag()
{
}

void AThread::start()
{
	assert(mThread == nullptr);
	auto t = shared_from_this();
    updateThreadName();
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
	if (t == nullptr) // abstract thread
	{
		t = _<AAbstractThread>(new CurrentThread(std::this_thread::get_id()));
	}

	return t;
}

void AThread::interruptionPoint()
{
	if (current()->isInterrupted())
	{
        current()->resetInterruptFlag();
		throw AInterrupted();
	}
}

bool AThread::isInterrupted()
{
	return mInterrupted;
}

void AThread::resetInterruptFlag()
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
    assert(("AAbstractThread::processMessages() should not be called from other thread",
            mId == std::this_thread::get_id()));
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


void AAbstractThread::setThreadName(const AString& name) {
    // stub
}

void AThread::setThreadName(const AString& name) {
    mThreadName = std::make_unique<AString>(name);
#ifdef _WIN32
    updateThreadName();
#else
    // TODO setThreadName for linux
#endif
}

void AThread::updateThreadName() {
#ifdef _WIN32
    if (mThreadName && mThread) {
        setThreadNameImpl((HANDLE) mThread->native_handle(), *mThreadName);
    }
#endif
}

AThread::AThread(std::function<void()> functor)
        : mFunctor(std::move(functor))
{
}
