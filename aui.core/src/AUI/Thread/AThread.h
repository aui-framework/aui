// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#pragma once
#include <thread>
#include <utility>
#include "AUI/Common/ADeque.h"
#include "AMutex.h"
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Common/AString.h"
#include <AUI/Platform/AStacktrace.h>
#include <functional>

class IEventLoop;
class AString;
class AConditionVariable;

/**
 * @brief Represents an abstract thread. Not all threads are created through AThread - these are interfaced with
 *        AAbstractThread.
 * @ingroup core
 */
class API_AUI_CORE AAbstractThread
{
	friend class IEventLoop;
	friend class AThread;
	friend class AConditionVariable;
    friend void setupUIThread() noexcept; // OSDesktop.cpp
public:

	/**
	 * @brief Thread ID type.
	 */
	typedef std::thread::id id;


public:
	/**
	 * @return thread ID
	 */
	id getId() const;

	/**
	 * @brief Delivers task for execution (message) to this thread's event queue. Messages are processed by framework
	 *        itself using AEventLoop. This behaviour may be overwritten using the <code>AThread::processMessages()
	 *        </code> function.
	 */
	void enqueue(std::function<void()> f);

	virtual ~AAbstractThread();

	/**
	 * @return true if interrupt requested for this thread.
	 */
	virtual bool isInterrupted();

	/**
	 * @brief Reset interruption flag.
	 */
	virtual void resetInterruptFlag();

    /**
     * @brief Interrupt thread's execution.
     * @details
     * Raises the interruption flag of the thread. In order to check whether thread interrupted use
     * AThread::interruptionPoint() or AAbstractThread::isInterrupted().
     */
    virtual void interrupt();

    /**
     * @brief Get current event loop for this thread.
     * @return current event loop for this thread
     */
	IEventLoop* getCurrentEventLoop() const {
		return mCurrentEventLoop;
	}


	/**
	 * @brief Enqueue message to make.
	 * @tparam Callable callable
	 * @param fun callable function
	 */
	template <class Callable>
    inline void operator<<(Callable fun) {
        enqueue(fun);
    }

    /**
     * @brief Enqueue message to make. Helper function for async, asyncX, ui, uiX
     * @tparam Callable callable
     * @param fun callable function
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

     /**
     * @brief Retrieve stacktrace of the thread.
     */
    [[nodiscard]]
    AStacktrace threadStacktrace() const;
        
	[[nodiscard]]
    bool messageQueueEmpty() noexcept;

protected:
    /**
     * @brief Thread ID.
     */
    id mId;

    AString mThreadName;

    /**
     * @brief Message queue mutex.
     */
    AMutex mQueueLock;

    struct Message {
        std::function<void()> proc;
    };

    /**
     * @brief Message queue.
     */
    ADeque<Message> mMessageQueue;

    AAbstractThread(const id& id) noexcept;
    void updateThreadName() noexcept;
    virtual void processMessagesImpl();


private:

    /**
     * @brief A condition variable that's currently locking the thread. Used for thread interruption.
     */
    struct {
        AMutex mutex;
        AConditionVariable* cv = nullptr;
    } mCurrentCV;

    /**
     * @brief Current IEventLoop for this thread. Used for inter thread message delivery.
     */
    IEventLoop* mCurrentEventLoop = nullptr;

    /**
     * @brief Mutex for mCurrentEventLoop.
     */
    AMutex mEventLoopLock;

    AAbstractThread() = default;


    /**
     * @brief AAbstractThread threadLocalStorage of current thread.
     */
    static _<AAbstractThread>& threadStorage();
};

#include "AUI/Common/AObject.h"

/**
 * @brief Represents a user-defined thread.
 * @ingroup core
 */
class API_AUI_CORE AThread : public AAbstractThread, public AObject
{
public:
	/**
	 * @brief Exception that is thrown by <code>AThread::interruptionPoint()</code>, if interruption is requested for
	 *        this thread. Handled by <code>AThread::start</code>.
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
	 * @brief Native thread handle.
	 */
	std::thread* mThread = nullptr;


	/**
	 * @brief Function that is called by <code>AThread::start()</code>. Becomes nullptr after call to
	 *        <code>AThread::start()</code>
	 */
	std::function<void()> mFunctor;

	/**
	 * @brief true if interrupt requested for this thread.
	 */
	bool mInterrupted = false;

public:

	AThread(std::function<void()> functor);

    virtual ~AThread();

    void detach();

	/**
	 * @brief Start thread execution.
	 */
	void start();


	/**
	 * @brief Sleep for specified duration.
	 *        Most operation systems guarantee that elasped time will be greater than specified.
	 *        <code>AThread::interrupt()</code> is supported.
	 * @param duration sleep duration.
	 */
	static void sleep(std::chrono::milliseconds duration);

	/**
	 * @return current thread.
	 */
	static _<AAbstractThread> current();

	/**
	 * @brief Interruption point
	 * @details
	 * If the interruption flag is raised for the caller thread then flag is reset and AThread::Interrupted exception is
	 * thrown, efficiently stopping the task execution and safely freeing resources with C++'s RAII feature.
	 *
	 * async, asyncX, AThreadPool::enqueue, AUI_ENTRY handle AThread::Interrupted, so throwing AThread::Interrupted is
	 * safe.
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
     * @brief Processes messages from other threads of current thread.
     * Called by framework itself using IEventLoop.
     */
    static void processMessages() {
        current()->processMessagesImpl();
    }

	bool isInterrupted() override;
	void resetInterruptFlag() override;
    void interrupt() override;

    /**
     * @brief Waits for thread to be finished.
     */
	void join();

};

#include "AConditionVariable.h"