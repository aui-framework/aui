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

#if AUI_COROUTINES

#include <coroutine>
#include "AUI/Common/AException.h"
#include "AUI/Thread/AAsyncHolder.h"
#include "AUI/Thread/AEventLoop.h"
#include "AUI/Thread/AThread.h"

#include <gtest/gtest.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>

static constexpr auto LOG_TAG = "CoroutinesTest";
using namespace std::chrono_literals;

namespace aui::impl::future {

/**
 * @brief Concept for types that satisfy the C++20 Awaiter requirements.
 * @details
 * An Awaiter must provide three methods:
 * - await_ready() returning a boolean
 * - await_suspend(std::coroutine_handle<>)
 * - await_resume()
 *
 * This is used to constrain await_transform overloads.
 */
template<typename T>
concept Cpp20Awaiter = requires(T& t) {
    { t.await_ready() } -> convertible_to<bool>;
    { t.await_suspend(std::coroutine_handle{}) };
    { t.await_resume() };
};

/**
 * @brief Concept for types that satisfy the C++20 Promise requirements.
 * @details
 * A Promise must provide:
 * - initial_suspend() returning an Awaiter
 * - final_suspend() returning an Awaiter
 * - get_return_object()
 */
template<typename T>
concept Cpp20Promise = requires(T& t) {
    { t.initial_suspend() } -> Cpp20Awaiter;
    { t.final_suspend() } -> Cpp20Awaiter;
    { t.get_return_object() };
};

/**
 * @brief Concept for types that have a nested promise_type satisfying Cpp20Promise.
 * @details
 * This is used to detect types that can be directly co_await ed (like AFuture<T>),
 * which have an associated promise_type that meets the C++20 coroutine requirements.
 */
template<typename T>
concept Cpp20HasPromiseType = requires {
    typename T::promise_type;
} && Cpp20Promise<typename T::promise_type>;

/**
 * @brief Promise type base for C++20 coroutines returning AFuture<T>.
 * @tparam T The value type of the AFuture<T> returned by the coroutine.
 * @details
 * CoPromiseTypeBase serves as the coroutine promise object for functions that return
 * AFuture<T>. It bridges the C++20 coroutine framework with AUI's AFuture mechanism,
 * allowing coroutines to seamlessly interoperate with the existing asynchronous future
 * infrastructure.
 *
 * The promise object is constructed when a coroutine function is called and manages the
 * coroutine's lifecycle, result delivery, exception propagation, and co_await integration.
 *
 */
template <typename T>
struct CoPromiseTypeBase {
    /**
     * @brief The future object that will be returned to the caller of the coroutine.
     * @details
     * This future is fulfilled when the coroutine completes (via return_value or return_void)
     * or when an unhandled exception occurs.
     */
    AFuture<T> future;

    /**
     * @brief Weak pointer to the future's inner shared state.
     * @details
     * Stored before moving the future out in get_return_object(), so the promise can
     * still access and fulfill the future after ownership has been transferred to the caller.
     * This is critical for safely handling cases where the caller has destroyed the future.
     */
    AFuture<T>::PtrWeak futureWeak = future.inner().weak();


    /**
     * @brief Called immediately by the compiler after the coroutine starts.
     * @return std::suspend_never, meaning the coroutine starts executing right away (not lazily).
     */
    auto initial_suspend() noexcept { return std::suspend_never {}; }

    /**
     * @brief Called by the compiler when the coroutine reaches its final suspension point.
     * @return std::suspend_never, meaning the coroutine frame is destroyed immediately after completion.
     */
    auto final_suspend() noexcept { return std::suspend_never {}; }

    /**
     * @brief Handles unhandled exceptions thrown within the coroutine body.
     * @details
     * Attempts to lock the weak pointer to the future's inner state. If the future is still alive, calls
     * `AFuture::supplyException()` to propagate the exception to whoever is awaiting the future. If the future has
     * already been destroyed, the exception is silently discarded.
     */
    auto unhandled_exception() noexcept {
        auto future = this->futureWeak.lock();
        if (future == nullptr) {
            return;
        }
        AFuture<T>(future).supplyException();
    }

    /**
     * @brief Called by the compiler to obtain the return value for the caller of the coroutine.
     * @return AFuture<T> that the caller receives. The future is moved out, leaving the
     *         internal future member in a moved-from state.
     */
    AFuture<T> get_return_object() noexcept {
#if AUI_DEBUG
        AUI_ASSERT(future.inner() != nullptr);
#endif
        auto toReturn = std::move(future);
#if AUI_DEBUG
        AUI_ASSERT(toReturn.inner() != nullptr);
        AUI_ASSERT(future.inner() == nullptr);
#endif
        return toReturn;
    }

    /**
     * @brief Overload of await_transform for types that are already Cpp20Awaiter.
     * @tparam NestedAwaiter An existing Awaiter type (e.g., from operator co_await).
     * @param nestedAwaiter The awaiter to wrap.
     * @return An Awaiter that delegates to the nested awaiter while adding cancellation
     *         detection via parentFuture.
     * @details
     * Wraps a nested Awaiter with cancellation support. Before resuming, checks if the
     * parent future has been destroyed (via parentFuture weak pointer). If so, throws
     * AThread::Interrupted to abort the coroutine gracefully instead of resuming into
     * a destroyed frame.
     */
    template <Cpp20Awaiter NestedAwaiter>
    auto await_transform(NestedAwaiter&& nestedAwaiter) {
        /**
         * @brief Awaiter that wraps a nested awaiter with cancellation detection.
         */
        struct Awaiter {
            NestedAwaiter nestedAwaiter;
            AFuture<T>::PtrWeak parentFuture;

            /** @brief Delegates to the nested awaiter's await_ready. */
            bool await_ready() noexcept {
                return nestedAwaiter.await_ready();
            }

            /**
             * @brief Resumes by delegating to the nested awaiter, but first checks
             *        if the parent future has been destroyed (cancellation check).
             * @throws AThread::Interrupted if the parent future was destroyed while suspended.
             */
            auto await_resume() {
                if (parentFuture.expired()) {
                    throw AThread::Interrupted{};
                }
                return nestedAwaiter.await_resume();
            }

            /**
             * @brief Delegates suspension to the nested awaiter.
             * @param ourHandle The coroutine handle for the parent coroutine.
             */
            auto await_suspend(std::coroutine_handle<typename Future<T>::CoPromiseType> ourHandle) {
                return nestedAwaiter.await_suspend(std::move(ourHandle));
            }
        };
        return Awaiter { .nestedAwaiter = std::forward<NestedAwaiter>(nestedAwaiter), .parentFuture = futureWeak };
    }

    /**
     * @brief Overload of await_transform for types that have a promise_type (like AFuture<T>).
     * @tparam NestedAwaitable An awaitable type (has promise_type), e.g. AFuture<F>.
     * @param nestedAwaitable The awaitable to transform.
     * @return An Awaiter (via the other overload) after converting via operator co_await.
     * @details
     * Converts the awaitable into an Awaiter via the free operator co_await function,
     * then delegates to the Cpp20Awaiter overload to wrap it with cancellation support.
     */
    template <Cpp20HasPromiseType NestedAwaitable>
    auto await_transform(NestedAwaitable&& nestedAwaitable) {
        return await_transform(operator co_await(std::forward<NestedAwaitable>(nestedAwaitable)));
    }

protected:
    /**
     * @brief Handles both return_void and return_value.
     *
     * This is a unified implementation used by both derived promise types. It locks the
     * weak pointer to the future and, if the future is still alive, calls `AFuture::supplyValue()` to propagate the
     * value to whoever is awaiting the future.
     *
     * @tparam F The value type(s). For AFuture<void>, this is an empty parameter pack.
     *           For AFuture<T>, this is a single forwarding reference to T.
     * @param v The value(s) to supply to the future.
     */
    template <typename... F>
    void return_(F&&... v) noexcept  {
        auto future = this->futureWeak.lock();
        if (future == nullptr) {
            return;
        }
        AFuture<T>(future).supplyValue(std::forward<F>(v)...);
    }
};
}

/**
 * @brief Promise type specialization for non-void AFuture<T> coroutines.
 * @tparam T The value type of the AFuture<T> returned by the coroutine.
 * @details
 * C++20 coroutine rules require a promise to provide either return_value or return_void,
 * but not both. This specialization provides return_value for non-void futures and
 * delegates to CoPromiseTypeBase::return_().
 */
template <typename T>
struct aui::impl::future::Future<T>::CoPromiseType: CoPromiseTypeBase<T> {
    using super = CoPromiseTypeBase<T>;

    // msvc is such a shitty compiler omg
    auto initial_suspend() noexcept {
        return super::initial_suspend();
    }

    auto final_suspend() noexcept {
        return super::final_suspend();
    }

    auto get_return_object() noexcept {
        return super::get_return_object();
    }

    auto unhandled_exception() noexcept {
        return super::unhandled_exception();
    }
    template<typename F>
    auto await_transform(F&& f) noexcept {
        return super::await_transform(std::forward<F>(f));
    }

    /**
     * @brief Called by the compiler when the coroutine co_return s a value.
     * @tparam F Forwarding reference to the returned value.
     * @param v The value to fulfill the future with.
     */
    template<typename F>
    void return_value(F&& v) noexcept  {
        super::return_(std::forward<F>(v));
    }
};

/**
 * @brief Promise type specialization for AFuture<void> coroutines.
 * @details
 * Provides return_void (required by the coroutine standard for void-returning promises)
 * and delegates to CoPromiseTypeBase::return_() with no arguments.
 */
template <>
struct aui::impl::future::Future<void>::CoPromiseType: CoPromiseTypeBase<void> {
    using super = CoPromiseTypeBase<void>;

    // msvc is such a shitty compiler omg
    auto initial_suspend() noexcept {
        return super::initial_suspend();
    }

    auto final_suspend() noexcept {
        return super::final_suspend();
    }

    auto get_return_object() noexcept {
        return super::get_return_object();
    }

    auto unhandled_exception() noexcept {
        return super::unhandled_exception();
    }
    template<typename F>
    auto await_transform(F&& f) noexcept {
        return super::await_transform(std::forward<F>(f));
    }

    /**
     * @brief Called by the compiler when the coroutine co_return s without a value.
     */
    void return_void() noexcept {
        this->return_();
    }
};

// Verify that AFuture<int> and AFuture<void> satisfy the Cpp20HasPromiseType concept.
static_assert(aui::impl::future::Cpp20HasPromiseType<AFuture<int>>);
static_assert(aui::impl::future::Cpp20HasPromiseType<AFuture<void>>);

/**
 * @brief Free operator co_await for AFuture<T>, enabling co_await on AFuture values.
 * @tparam T The value type of the future.
 * @param future The AFuture<T> to await.
 * @return An Awaiter that integrates with the coroutine suspension machinery.
 * @details
 * This is the primary mechanism by which AFuture<T> becomes awaitable. The returned Awaiter:
 *
 * - **await_ready**: Returns true if the future already has a result (no suspension).
 * - **await_resume**: Dereferences the future to obtain the value (or void).
 * - **await_suspend**: Registers onSuccess/onError callbacks on the future. When the
 *   future completes (possibly on a different thread), the callback enqueues resumption
 *   onto the **caller's original thread** via AThread::current()->enqueue(). This ensures
 *   the coroutine resumes on the same thread that initiated the co_await, avoiding deep
 *   call stacks and inconsistent threading.
 */
template <typename T>
auto operator co_await(AFuture<T> future) {
    struct Awaiter {
        AFuture<T> future;

        bool await_ready() const noexcept { return future.hasResult(); }

        auto await_resume() {
            if constexpr (std::is_same_v<T, void>) {
                *future;
            } else {
                return std::move(*future);
            }
        }

        void await_suspend(std::coroutine_handle<> handle) {
            // onSuccess and onError are called by supplyValue which might have been called on a different thread.
            // if we keep this logic for coroutines, the callstack will grow very fast and inconsistent.
            // for co_await, let's keep the caller thread.
            // if user wants to magically switch threads using co_await, they'll figure out how to shoot their knee.
            auto callback = [handle = std::move(handle), callerThread = AThread::current()](const auto&...) {
                callerThread->enqueue([handle = std::move(handle)] {
                    handle.resume();
                });
            };
            future.onSuccess(callback);
            future.onError(std::move(callback));
        }
    };
    return Awaiter{ std::move(future) };
}


/**
 * @brief Helper function that returns a future completing after 500ms with value 228.
 * @return AFuture<int> that resolves to 228.
 */
static AFuture<int> longTask() {
    return AUI_THREADPOOL {
        AThread::sleep(500ms); // long tamssk
        return 228;
    };
}

/**
 * @brief Helper function that returns a future completing after 500ms with an exception.
 * @return AFuture<int> that resolves with AException.
 */
static AFuture<int> longTaskException() {
    return AUI_THREADPOOL -> int {
        AThread::sleep(500ms); // long tamssk
        throw AException("Whoops! Something bad happened");
    };
}

TEST(Coroutines, CoAwaitCancellation) {
    auto testArgumentCapture = _new<int>(322);
    {
        auto goesOutOfScope = [](_<int> testArgumentCapture) -> AFuture<> {
            co_await longTask();
            *testArgumentCapture = 0;
            ADD_FAILURE() << "should be dead already";
        }(testArgumentCapture);
    }

    AEventLoop loop;
    IEventLoop::Handle h(&loop);
    loop.iteration();
    AThread::processMessages();
    EXPECT_EQ(testArgumentCapture.use_count(), 1) << "testArgumentCapture was not cleaned from coro frame";
}

TEST(Coroutines, CoAwaitCancellationException) {
    auto testArgumentCapture = _new<int>(322);
    {
        auto goesOutOfScope = [](_<int> testArgumentCapture) -> AFuture<> {
            co_await longTaskException();
            *testArgumentCapture = 0;
            ADD_FAILURE() << "should be dead already";
        }(testArgumentCapture);
    }

    AEventLoop loop;
    IEventLoop::Handle h(&loop);
    loop.iteration();
    AThread::processMessages();
    EXPECT_EQ(testArgumentCapture.use_count(), 1) << "testArgumentCapture was not cleaned from coro frame";
}

TEST(Coroutines, CoAwaitCorruptionTest) {
    AEventLoop loop;
    IEventLoop::Handle h(&loop);
    AAsyncHolder async;

    async << [](_<int> testArgumentCapture) -> AFuture<> {
        co_await AThread::asyncSleep(500ms);
        EXPECT_EQ(*testArgumentCapture, 322) << "argument capture is corrupted";
    }(_new<int>(322));

    auto messWithHeap = [] {
        for (int i = 0; i < 1024; ++i) {
            std::vector<int> zeroes(i);
            AUI_NO_OPTIMIZE_OUT(zeroes);
        }
    };
    messWithHeap();
    loop.iteration();
    messWithHeap();
    AThread::processMessages();
    messWithHeap();
    AThread::processMessages();
}

TEST(Coroutines, CoAwait) {
    /// [co_await1]
    AAsyncHolder async;
    async << []() -> AFuture<> {
        auto v228 = co_await longTask();
        EXPECT_EQ(v228, 228);
        co_return;
    }();

    AEventLoop loop;
    IEventLoop::Handle h(&loop);
    while (async.size() > 0) {
        loop.iteration();
    }
    /// [co_await1]
}


TEST(Coroutines, CoAwaitException) {
    AAsyncHolder async;
    auto future = []() -> AFuture<> {
        try {
            auto v228 = co_await longTaskException();
        } catch (const AException& e) {
            co_return;
        }
        ADD_FAILURE() << "exception was not reported";
    }();


    AEventLoop loop;
    IEventLoop::Handle h(&loop);
    while (async.size() > 0) {
        loop.iteration();
    }
}

TEST(Coroutines, CoAwaitPreservesThread) {
    /// [co_await1]
    AAsyncHolder async;
    async << []() -> AFuture<> {
        auto original = AThread::current();
        co_await longTask();
        EXPECT_EQ(AThread::current(), original);
    }();

    AEventLoop loop;
    IEventLoop::Handle h(&loop);
    while (async.size() > 0) {
        loop.iteration();
    }
    /// [co_await1]
}

static auto switch_to_new_thread(std::thread& out)
{
    // https://en.cppreference.com/cpp/language/coroutines
    struct awaitable
    {
        std::thread* p_out;
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h)
        {
            std::thread& out = *p_out;
            if (out.joinable())
                throw std::runtime_error("Output thread parameter not empty");
            out = std::thread([h] { h.resume(); });
        }
        void await_resume() {}
    };
    return awaitable{&out};
}

TEST(Coroutines, CoAwait3rdPartyCoro) {
    AAsyncHolder async;
    std::thread thread;
    auto future = [](std::thread& thread) -> AFuture<int> {
        auto original = AThread::current();
        co_await switch_to_new_thread(thread);
        EXPECT_NE(AThread::current(), original);
        co_return 123;
    }(thread);
    async << future;


    AEventLoop loop;
    IEventLoop::Handle h(&loop);
    while (async.size() > 0) {
        loop.iteration();
    }
    thread.join();
    EXPECT_EQ(*future, 123) << "return value was not delivered correctly";
}



#endif
