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

#include "AThread.h"

#include <cassert>
#include <AUI/Common/AException.h>
#include <AUI/Common/AString.h>
#include <AUI/Logging/ALogger.h>

#include "IEventLoop.h"
#include <AUI/Thread/AConditionVariable.h>

#if AUI_PLATFORM_WIN
#include <windows.h>


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
	auto t = _cast<AThread>(objectSharedPtr());
    updateThreadName();
	mThread = new std::thread([&, t] ()
	{
		threadStorage() = t;
		auto f = std::move(mFunctor);
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
	std::unique_lock lock(mCurrentCV.mutex);
    nullsafe(mCurrentCV.cv)->notify_all();
}

void AThread::sleep(unsigned durationInMs)
{
    AConditionVariable cv;
    AMutex mutex;
    std::unique_lock lock(mutex);
    // the condition variable helps with thread interruption.
    cv.wait_for(lock, std::chrono::milliseconds(durationInMs));
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
		t = aui::ptr::manage(new CurrentThread(std::this_thread::get_id()));
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
	if (mThread->joinable()) mThread->join();
}

void AAbstractThread::enqueue(std::function<void()> f)
{
	{
		std::unique_lock lock(mQueueLock);
		mMessageQueue << std::move(f);
	}
	{
		if (mCurrentEventLoop) {
			std::unique_lock lock(mEventLoopLock);
			if (mCurrentEventLoop)
			{
				mCurrentEventLoop->notifyProcessMessages();
			}
		}
	}
}

void AAbstractThread::processMessagesImpl()
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

void AThread::detach() {
    mThread->detach();
    delete mThread;
    mThread = nullptr;
}



void AAbstractThread::updateThreadName() noexcept {
    if (!mThreadName.empty()) {
#if AUI_PLATFORM_WIN
        setThreadNameImpl((HANDLE) GetCurrentThread(), mThreadName);
#elif AUI_PLATFORM_APPLE
        auto name = mThreadName->toStdString();
        assert(("on unix thread name restricted to 15 chars length", name.size() < 16));
        pthread_setname_np(name.c_str());
#else
        auto name = mThreadName.toStdString();
        assert(("on unix thread name restricted to 15 chars length", name.size() < 16));
        pthread_setname_np(pthread_self(), name.c_str());
#endif
    }
}

AThread::AThread(std::function<void()> functor)
        : mFunctor(std::move(functor))
{
}
