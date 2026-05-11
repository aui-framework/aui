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
        } catch (const AException &e) {
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

static auto switch_to_new_thread(std::thread &out) {
    // https://en.cppreference.com/cpp/language/coroutines
    struct awaitable {
        std::thread *p_out;

        bool await_ready() { return false; }

        void await_suspend(std::coroutine_handle<> h) {
            std::thread &out = *p_out;
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
    auto future = [](std::thread &thread) -> AFuture<int> {
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
