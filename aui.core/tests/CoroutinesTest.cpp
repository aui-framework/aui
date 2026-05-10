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

#include <gtest/gtest.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>

static constexpr auto LOG_TAG = "CoroutinesTest";
using namespace std::chrono_literals;

namespace aui::impl::future {

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
        AFuture(future).supplyException();
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
     * @brief Called by the compiler when co_await is used within a coroutine returning AFuture<T>.
     * @tparam F The value type of the awaited AFuture<F>.
     * @param childCoro The future being awaited via co_await.
     * @return An Awaitable object that the coroutine framework uses to manage suspension.
     * @details
     * Transforms an awaited AFuture<F> into an Awaitable that:
     *
     * - Checks if the child future is already ready (`await_ready`).
     * - Retrieves the result (or void) from the child future (`await_resume`).
     * - Registers callbacks on the child future that resume our coroutine on the
     *   original caller thread when the child completes (`await_suspend`).
     *
     * The resumption is always enqueued onto the caller's original thread to avoid deep
     * call stacks and inconsistent threading. If our future has been destroyed
     * before the child completes, the coroutine frame is safely destroyed to prevent
     * use-after-free.
     *
     */
    template <typename F>
    auto await_transform(AFuture<F>&& childCoro) {
        /**
         * @brief Awaitable object returned by await_transform.
         */
        struct Awaitable {
            AFuture<F> childCoro;

            /** @brief Returns true if the child future already has a result (no suspension needed). */
            bool await_ready() const noexcept { return childCoro.hasResult(); }

            /**
             * @brief Called after the coroutine resumes to obtain the awaited value.
             * @return The value from the child future, or void if F is void.
             */
            auto await_resume() {
                if constexpr (std::is_same_v<F, void>) {
                    *childCoro;
                } else {
                    return std::move(*childCoro);
                }
            }

            /**
             * @brief Called when the coroutine must suspend waiting for the child future.
             *
             * Registers onSuccess/onError callbacks on the child future. When the child
             * completes (possibly on a different thread), the callback enqueues resumption
             * of the parent coroutine onto the caller's original thread. If the parent
             * future has been destroyed, the coroutine frame is destroyed instead.
             *
             * @param handle The coroutine handle for the parent coroutine (this coroutine).
             */
            void await_suspend(std::coroutine_handle<typename Future<T>::CoPromiseType> ourHandle) {
                // onSuccess and onError are called by supplyValue which might have been called on a different thread.
                // if we keep this logic for coroutines, the callstack will grow very fast and inconsistent.
                // for co_await, let's keep the caller thread.
                // if user wants to magically switch threads using co_await, they'll figure out how to shoot their knee.
                auto callback = [ourHandle = std::move(ourHandle), callerThread = AThread::current()](const auto&...) {
                    callerThread->enqueue([ourHandle = std::move(ourHandle)] {
                        typename Future<T>::CoPromiseType& promise = ourHandle.promise();
                        auto parentCoroLock = promise.futureWeak.lock();
                        if (parentCoroLock) {
                            ourHandle.resume();
                        } else {
                            // parent coro destroyed. continuing executing coro might lead to segfaults.
                            // we'll destroy the coroutine.
                            // if the user still wants the coroutine to live, they should prolong parent AFuture
                            // lifetime.
                            ourHandle.destroy();
                        }
                    });
                };
                childCoro.onSuccess(callback);
                childCoro.onError(std::move(callback));
            }
        };
        return Awaitable { .childCoro = std::move(childCoro) };
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
        AFuture(future).supplyValue(std::forward<F>(v)...);
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
    /**
     * @brief Called by the compiler when the coroutine co_return s a value.
     * @tparam F Forwarding reference to the returned value.
     * @param v The value to fulfill the future with.
     */
    template<typename F>
    void return_value(F&& v) noexcept  {
        this->return_(std::forward<F>(v));
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
    /**
     * @brief Called by the compiler when the coroutine co_return s without a value.
     */
    void return_void() noexcept {
        this->return_();
    }
};

// template<typename T>
// struct Awaitable {
//     AFuture<T> future;
//
//     bool await_ready() const noexcept { return future.hasResult(); }
//
//     auto await_resume() {
//         if constexpr (std::is_same_v<T, void>) {
//             *future;
//         } else {
//             return std::move(*future);
//         }
//     }
//
//     template<typename NestedCoroutinePromise>
//     void await_suspend(std::coroutine_handle<NestedCoroutinePromise> handle) {
//         // onSuccess and onError are called by supplyValue which might have been called on a different thread.
//         // if we keep this logic for coroutines, the callstack will grow very fast and inconsistent.
//         // for co_await, let's keep the caller thread.
//         // if user wants to magically switch threads using co_await, they'll figure out how to shoot their knee.
//         auto callback = [handle = std::move(handle), callerThread = AThread::current()](const auto&...) {
//             callerThread->enqueue([handle = std::move(handle)] {
//                 handle.resume();
//             });
//         };
//         future.onSuccess(callback);
//         future.onError(std::move(callback));
//     }
// };
//
// template <typename T>
// auto operator co_await(AFuture<T> future) {
//
//     return Awaitable{ std::move(future) };
// }


static AFuture<int> longTask() {
    return AUI_THREADPOOL {
        AThread::sleep(500ms); // long tamssk
        return 228;
    };
}

TEST(Coroutines, CoAwaitCancellation) {
    auto testArgumentCapture = _new<int>(322);
    {
        auto goesOutOfScope = [](_<int> testArgumentCapture) -> AFuture<> {
            co_await longTask();
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

static AFuture<int> longTaskException() {
    return AUI_THREADPOOL -> int {
        AThread::sleep(10ms); // long tamssk
        throw AException("Whoops! Something bad happened");
    };
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

#endif
