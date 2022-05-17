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
#include <thread>
#include <utility>
#include "AUI/Common/ADeque.h"
#include "AMutex.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Common/AString.h"
#include <functional>

class IEventLoop;
class AString;
class AConditionVariable;

/**
 * \brief Abstract thread. Not all threads are created through AThread - these are interfaced with AAbstractThread.
 */
class API_AUI_CORE AAbstractThread
{
	friend class IEventLoop;
	friend class AThread;
	friend class AConditionVariable;
public:

	/**
	 * \brief Thread ID type.
	 */
	typedef std::thread::id id;

private:
	/**
	 * \brief Message queue.
	 */
	ADeque<std::function<void()>> mMessageQueue;

	/**
	 * \brief Message queue mutex.
	 */
	AMutex mQueueLock;

	/**
	 * \brief Thread ID.
	 */
	id mId;

    /**
     * \brief A condition variable that's currently locking the thread. Used for thread interruption.
     */
	struct {
        AMutex mutex;
        AConditionVariable* cv = nullptr;
    } mCurrentCV;
	
	/**
	 * \brief Current IEventLoop for this thread. Used for inter thread message delivery.
	 */
	IEventLoop* mCurrentEventLoop = nullptr;

    /**
     * \brief Mutex for mCurrentEventLoop.
     */
    AMutex mEventLoopLock;

	AAbstractThread() = default;
	AAbstractThread(const id& id);


	/**
	 * \brief AAbstractThread threadLocalStorage of current thread.
	 */
	static _<AAbstractThread>& threadStorage();

public:
	/**
	 * \return thread ID
	 */
	id getId() const;

	/**
	 * \brief Delivers task for execution (message) to this thread's event queue. Messages are processed by framework
	 *        itself using AEventLoop. This behaviour may be overwritten using the <code>AThread::processMessages()
	 *        </code> function.
	 */
	void enqueue(std::function<void()> f);

	virtual ~AAbstractThread();

	/**
	 * \return true if interrupt requested for this thread.
	 */
	virtual bool isInterrupted();

	/**
	 * \brief Reset interruption flag.
	 */
	virtual void resetInterruptFlag();

    /**
     * \brief Interrupt thread's execution.
     *        This function requires the interrupted code contain calls to the <code>AThread::interruptionPoint()</code>
     *        function since C++ is native programming language (not managed)
     */
    virtual void interrupt();

    /**
     * \brief Get current event loop for this thread.
     * \return current event loop for this thread
     */
	IEventLoop* getCurrentEventLoop() const {
		return mCurrentEventLoop;
	}


	/**
	 * \brief Enqueue message to make.
	 * \tparam Callable callable
	 * \param fun callable function
	 */
	template <class Callable>
    inline void operator<<(Callable fun) {
        enqueue(fun);
    }

    /**
     * \brief Enqueue message to make. Helper function for async, asyncX, ui, uiX
     * \tparam Callable callable
     * \param fun callable function
     */
    template <class Callable>
    inline void operator*(Callable fun)
    {
        enqueue(fun);
    }

    [[nodiscard]]
    const AString& threadName() const noexcept {
        return mThreadName;
    }

protected:
    AString mThreadName;

    void updateThreadName() noexcept;
    void processMessagesImpl();
};

#include "AUI/Common/AObject.h"

/**
 * \brief Thread.
 */
class API_AUI_CORE AThread : public AAbstractThread, public AObject
{
public:
	/**
	 * \brief Exception that is thrown by <code>AThread::interruptionPoint()</code>, if interruption is requested for
	 *        this thread. Handled by <code>AThread::start</code.
	 */
	class Interrupted
	{
    public:
        /**
         * @brief Schedules AThread::Interrupted exception to the next interruption point.
         * Sometimes you could not throw exceptions (i.e. in a noexcept function or destructor). In this case
         * you may call needRethrow in order to throw Interrupted exception at the next interruption point.
         */
        void needRethrow() const noexcept {
            AThread::current()->interrupt();
        }
	};

private:
	/**
	 * \brief Native thread handle.
	 */
	std::thread* mThread = nullptr;


	/**
	 * \brief Function that is called by <code>AThread::start()</code>. Becomes nullptr after call to
	 *        <code>AThread::start()</code>
	 */
	std::function<void()> mFunctor;

	/**
	 * \brief true if interrupt requested for this thread.
	 */
	bool mInterrupted = false;

public:

	AThread(std::function<void()> functor);

    virtual ~AThread();

    void detach();

	/**
	 * \brief Start thread execution.
	 */
	void start();


	/**
	 * \brief Sleep for specified duration.
	 *        Most operation systems guarantee that elasped time will be greater than specified.
	 *        <code>AThread::interrupt()</code> is supported.
	 * \param durationInMs duration in milliseconds.
	 */
	static void sleep(unsigned durationInMs);

	/**
	 * \return current thread.
	 */
	static _<AAbstractThread> current();

	/**
	 * \brief Interruption point. It's required for <code>AThread::interrupt</code>.
	 */
	static void interruptionPoint();



    /**
     * Sets name of the current thread for debugger.
     * @param name new name of the thread
     */
    static void setName(AString name) noexcept {
        auto cur = current();
        cur->mThreadName = std::move(name);
        cur->updateThreadName();
    }
    /**
     * \brief Processes messages from other threads of current thread.
     * Called by framework itself using IEventLoop.
     */
    static void processMessages() {
        current()->processMessagesImpl();
    }

	bool isInterrupted() override;
	void resetInterruptFlag() override;
    void interrupt() override;

    /**
     * \brief Waits for thread to be finished.
     */
	void join();

};

#include "AConditionVariable.h"