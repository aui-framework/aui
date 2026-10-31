/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <thread>
#include <utility>
#include <functional>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AString.h>
#include <AUI/Common/AObject.h>
#include <AUI/Platform/AStacktrace.h>
#include <AUI/Thread/AMutex.h>
#include <AUI/Util/AMessageQueue.h>
#include <AUI/Traits/concepts.h>

#if AUI_COROUTINES
#include <coroutine>
#endif

class IEventLoop;
class AString;
class AConditionVariable;

template<typename T = void>
class AFuture;

namespace aui::impl::future {
template <typename T>
struct Unwrap {
    using type = T;
};

template <typename T>
struct Unwrap<AFuture<T>> {
    using type = T;
};
}   // namespace aui::impl::future

/**
 * @brief Represents an abstract thread which might be not created with AThread.
 * @ingroup core
 * @details
 * Not all threads are created through AThread - these are interfaced with AAbstractThread.
 *
 * See also: `AThread::current()`.
 */
class API_AUI_CORE AAbstractThread {
    friend class IEventLoop;
    friend class AThread;
    friend class AConditionVariable;
    friend void setupUIThread() noexcept;   // OSDesktop.cpp

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
    void enqueue(AMessageQueue<>::Message f);

    [[nodiscard]]
    const AMessageQueue<>& messageQueue() const {
        return mMessageQueue;
    }

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
    IEventLoop* getCurrentEventLoop() const { return mCurrentEventLoop; }

    /**
     * @brief Notifies the current event loop of this thread (if any) that there are messages to process.
     * @details
     * Uses the same double-checked locking as enqueue() to avoid a race with IEventLoop::Handle
     * destructor nulling out mCurrentEventLoop. Safe to call from any thread.
     */
    void notifyCurrentEventLoop() noexcept;

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
     * @brief Enqueue message to make. Helper function for AUI_THREADPOOL, AUI_THREADPOOL_X, ui, uiX
     * @tparam Callable callable
     * @param fun callable function
     */
    template <class Callable>
    inline void operator*(Callable fun) {
        enqueue(fun);
    }

    /**
     * @brief Runs `fn` on this thread and returns the future of its result.
     * @details
     * `fn` runs when this thread processes its messages, never inline, even when called from this very thread. An
     * exception thrown by `fn` fails the future. If `fn` returns an `AFuture<T>`, the result is `AFuture<T>` that
     * completes with it, not `AFuture<AFuture<T>>`.
     *
     * ```cpp
     * AFuture<AString> title = AThread::main()->async([this] { return mLabel->text(); });
     * ```
     *
     * If the future is cancelled before this thread gets to `fn`, `fn` is not called. If the thread stops without
     * processing the message, the future fails with [AFutureAbandonedException].
     *
     * !!! warning
     *
     *     Don't block on the returned future on this same thread: the message can't be processed while the thread
     *     waits for it. In a coroutine, `co_await` it instead, or continue on this thread with `switchTo`.
     */
    template <aui::invocable Fn>
    auto async(Fn fn) -> AFuture<typename aui::impl::future::Unwrap<std::invoke_result_t<Fn&>>::type>;

#if AUI_COROUTINES
    /**
     * @brief Continues the coroutine on this thread: `co_await AThread::main()->switchTo();`.
     * @details
     * Suspends the coroutine and posts its resumption to this thread with `enqueue`, the same way as
     * `AUI_UI_THREAD { ... }` posts a lambda. The coroutine continues when this thread processes its messages: the UI
     * thread does it in its event loop; a thread of your own needs an [AEventLoop] (or calls
     * `AThread::processMessages()`). Does not suspend if the coroutine is on this thread already. The coroutine stays
     * on this thread until the next switch: a following `co_await future` resumes here too.
     *
     * ```cpp
     * AFuture<> MyView::reload() {
     *     co_await mDatabaseThread->switchTo();
     *     auto title = mDatabase->queryTitle();  // blocking, on mDatabaseThread
     *     co_await AThread::main()->switchTo();
     *     setText(title);                        // UI thread
     * }
     * ```
     *
     * If the thread never processes its messages, the coroutine is never resumed. If the thread stops without
     * processing the message, the coroutine is destroyed; a coroutine returning AFuture then fails with
     * [AFutureAbandonedException].
     */
    [[nodiscard]]
    auto switchTo();
#endif

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

    AMessageQueue<> mMessageQueue;

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

/**
 * @brief Represents a user-defined thread.
 * @ingroup core
 */
class API_AUI_CORE AThread : public AAbstractThread, public AObject {
public:
    /**
     * @brief Exception that is thrown by <code>AThread::interruptionPoint()</code>, if interruption is requested for
     *        this thread. Handled by <code>AThread::start</code>.
     */
    class Interrupted {
    public:
        /**
         * @brief Schedules AThread::Interrupted exception to the next interruption point.
         * Sometimes you could not throw exceptions (i.e. in a noexcept function or destructor). In this case
         * you may call needRethrow in order to throw Interrupted exception at the next interruption point.
         */
        void needRethrow() const noexcept { AThread::current()->interrupt(); }
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
    std::atomic_bool mInterrupted = false;

public:
    AThread(std::function<void()> functor);

    ~AThread() override;

    void detach();

    /**
     * @brief Returns main thread of the application.
     */
    [[nodiscard]]
    static const _<AAbstractThread>& main() noexcept;

    /**
     * @brief Start thread execution.
     */
    void start();

    /**
     * @brief Sleep for specified duration.
     *        Most operation systems guarantee that elasped time will be greater than specified.
     *        <code>AThread::interrupt()</code> is supported.
     * @param duration sleep duration.
     * @details
     * Blocking sleep.
     */
    static void sleep(std::chrono::milliseconds duration);

    /**
     * @brief Sleep for specified duration.
     *        Most operation systems guarantee that elasped time will be greater than specified.
     *        <code>AThread::interrupt()</code> is supported.
     * @param duration sleep duration.
     * @details
     * Async sleep.
     *
     * ```cpp
     * co_await AThread::asyncSleep(1s);
     * ```
     */
    static AFuture<> asyncSleep(std::chrono::milliseconds duration);

    /**
     * @return current thread.
     */
    [[nodiscard]]
    static const _<AAbstractThread>& current();

    /**
     * @brief Interruption point
     * @details
     * If the interruption flag is raised for the caller thread then flag is reset and AThread::Interrupted exception is
     * thrown, efficiently stopping the task execution and safely freeing resources with C++'s RAII feature.
     *
     * AUI_THREADPOOL, AUI_THREADPOOL_X, AThreadPool::enqueue, AUI_ENTRY handle AThread::Interrupted, so throwing AThread::Interrupted is
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
    static void processMessages() { current()->processMessagesImpl(); }

    bool isInterrupted() override;
    void resetInterruptFlag() override;
    void interrupt() override;

    /**
     * @brief Waits for thread to be finished.
     */
    void join();
};

#if AUI_COROUTINES
namespace aui::impl {
struct ThreadSwitchAwaiter {
    AAbstractThread* thread;

    [[nodiscard]]
    bool await_ready() const noexcept {
        return thread == AThread::current().get();
    }

    void await_suspend(std::coroutine_handle<> handle) const {
        struct Resume {
            std::coroutine_handle<> handle;

            ~Resume() {
                // the message is dropped without a call (the thread has stopped): don't leak the coroutine
                if (handle) {
                    handle.destroy();
                }
            }
        };
        // std::function requires a copyable callable
        thread->enqueue([resume = std::make_shared<Resume>(handle)] { std::exchange(resume->handle, nullptr).resume(); });
    }

    void await_resume() const noexcept {}
};
}   // namespace aui::impl

inline auto AAbstractThread::switchTo() {
    return aui::impl::ThreadSwitchAwaiter { this };
}
#endif
