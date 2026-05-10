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
template <typename T>
struct CoPromiseTypeBase {
    AFuture<T> future;
    AFuture<T>::PtrWeak futureWeak = future.inner().weak();


    auto initial_suspend() noexcept { return std::suspend_never {}; }

    auto final_suspend() noexcept { return std::suspend_never {}; }

    auto unhandled_exception() noexcept {
        auto future = this->futureWeak.lock();
        if (future == nullptr) {
            return;
        }
        AFuture(future).supplyException();
    }

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
     * @brief Called by the compiler when co_await operator is used from within a function that returns AFuture<T>.
     */
    template <typename F>
    auto await_transform(AFuture<F>&& childCoro) {
        struct Awaitable {
            AFuture<F> childCoro;

            bool await_ready() const noexcept { return childCoro.hasResult(); }

            auto await_resume() {
                if constexpr (std::is_same_v<F, void>) {
                    *childCoro;
                } else {
                    return std::move(*childCoro);
                }
            }

            void await_suspend(std::coroutine_handle<typename Future<T>::CoPromiseType> handle) {
                // onSuccess and onError are called by supplyValue which might have been called on a different thread.
                // if we keep this logic for coroutines, the callstack will grow very fast and inconsistent.
                // for co_await, let's keep the caller thread.
                // if user wants to magically switch threads using co_await, they'll figure out how to shoot their knee.
                auto callback = [handle = std::move(handle), callerThread = AThread::current()](const auto&...) {
                    callerThread->enqueue([handle = std::move(handle)] {
                        typename Future<T>::CoPromiseType& promise = handle.promise();
                        auto parentCoroLock = promise.futureWeak.lock();
                        if (parentCoroLock) {
                            handle.resume();
                        } else {
                            // parent coro destroyed. continuing executing coro might lead to segfaults.
                            // we'll destroy the coroutine.
                            // if the user still wants the coroutine to live, they should prolong parent AFuture
                            // lifetime.
                            handle.destroy();
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
     * @tparam F T of AFuture<T>. In case of AFuture<void>, empty parameter pack.
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

// template generalizations to handle return_valie and return_void, respectively.
// we can't provide both simultaniously.
// we'll dispatch void and actual value in CoPromiseTypeBase::return_.
template <typename T>
struct aui::impl::future::Future<T>::CoPromiseType: CoPromiseTypeBase<T> {
    template<typename F>
    void return_value(F&& v) noexcept  {
        this->return_(std::forward<F>(v));
    }
};

template <>
struct aui::impl::future::Future<void>::CoPromiseType: CoPromiseTypeBase<void> {
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
