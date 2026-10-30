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

#include <gtest/gtest.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Thread/AEventLoop.h>
#include <AUI/Util/kAUI.h>

namespace {

/// A thread that processes its messages until destroyed.
class LoopThread {
public:
    LoopThread() {
        mThread = _new<AThread>([this] {
            AEventLoop loop;
            IEventLoop::Handle h(&loop);
            while (!mStop) {
                loop.iteration();
            }
        });
        mThread->start();
    }

    ~LoopThread() {
        mThread->enqueue([this] { mStop = true; });
        mThread->join();
    }

    [[nodiscard]]
    AAbstractThread* get() const noexcept {
        return mThread.get();
    }

    AAbstractThread* operator->() const noexcept { return get(); }

private:
    std::atomic_bool mStop = false;
    _<AThread> mThread;
};

AString rootMessage(const AException& e) {
    std::exception_ptr cause = e.causedBy();
    AString message = e.getMessage();
    while (cause) {
        try {
            std::rethrow_exception(cause);
        } catch (const AException& inner) {
            message = inner.getMessage();
            cause = inner.causedBy();
        } catch (...) {
            break;
        }
    }
    return message;
}

}   // namespace

TEST(ThreadAsync, RunsOnTheThreadAndReturnsTheResult) {
    LoopThread worker;
    auto future = worker->async([] { return AThread::current().get(); });
    EXPECT_EQ(*future, worker.get());
}

TEST(ThreadAsync, NeverRunsInline) {
    auto future = AThread::current()->async([] { return 42; });
    EXPECT_FALSE(future.hasResult());
    AThread::processMessages();
    ASSERT_TRUE(future.hasValue());
    EXPECT_EQ(*future, 42);
}

TEST(ThreadAsync, Void) {
    LoopThread worker;
    std::atomic_int calls = 0;
    auto future = worker->async([&] { ++calls; });
    static_assert(std::is_same_v<decltype(future), AFuture<>>);
    future.wait();
    EXPECT_EQ(calls, 1);
}

TEST(ThreadAsync, ExceptionFailsTheFuture) {
    LoopThread worker;
    auto future = worker->async([]() -> int { throw AException("nope"); });
    try {
        *future;
        FAIL();
    } catch (const AException& e) {
        EXPECT_EQ(rootMessage(e), "nope");
    }
}

TEST(ThreadAsync, MoveOnlyCallable) {
    LoopThread worker;
    auto future = worker->async([value = std::make_unique<int>(7)] { return *value; });
    EXPECT_EQ(*future, 7);
}

TEST(ThreadAsync, CancelledBeforeTheThreadGetsToItIsNotCalled) {
    bool called = false;
    auto future = AThread::current()->async([&] { called = true; });
    future.cancel();
    AThread::processMessages();
    EXPECT_FALSE(called);
}

TEST(ThreadAsync, FutureResultIsFlattened) {
    LoopThread worker;
    AOptional<AFuture<int>::Resolver> resolver;
    auto future = worker->async([&] {
        return AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); });
    });
    static_assert(std::is_same_v<decltype(future), AFuture<int>>);
    while (!resolver) {
        AThread::sleep(std::chrono::milliseconds(1));
    }
    EXPECT_FALSE(future.hasResult());
    resolver->supplyValue(5);
    EXPECT_EQ(*future, 5);
}

TEST(ThreadAsync, FlattenedExceptionKeepsTheCause) {
    auto future = AThread::current()->async([] {
        return AFuture<int>::fromException(std::make_exception_ptr(AException("inner")));
    });
    AThread::processMessages();
    try {
        *future;
        FAIL();
    } catch (const AException& e) {
        EXPECT_EQ(rootMessage(e), "inner");
    }
}

TEST(ThreadAsync, DroppingTheResultCancelsTheFlattenedFuture) {
    AOptional<AFuture<int>::Resolver> resolver;
    {
        auto future = AThread::current()->async([&] {
            return AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); });
        });
        AThread::processMessages();
        ASSERT_TRUE(resolver);
        EXPECT_FALSE(resolver->isCancelled());
    }
    EXPECT_TRUE(resolver->isCancelled());
}

TEST(ThreadAsync, CancelledFlattenedFutureFailsTheResult) {
    AOptional<AFuture<int>> inner;
    AOptional<AFuture<int>::Resolver> resolver;
    auto future = AThread::current()->async([&] {
        inner = AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); });
        return *inner;
    });
    AThread::processMessages();
    inner->cancel();
    try {
        *future;
        FAIL();
    } catch (const AException& e) {
        EXPECT_EQ(rootMessage(e), AFutureAbandonedException().getMessage());
    }
}

#if AUI_COROUTINES

namespace {
AFuture<> hop(AAbstractThread* home, AAbstractThread* worker, std::vector<AAbstractThread*>& visited) {
    visited.push_back(AThread::current().get());
    co_await worker->switchTo();
    visited.push_back(AThread::current().get());
    co_await home->switchTo();
    visited.push_back(AThread::current().get());
}
}   // namespace

TEST(ThreadSwitch, ResumesOnTheThread) {
    AEventLoop loop;
    IEventLoop::Handle h(&loop);
    LoopThread worker;
    auto home = AThread::current().get();
    std::vector<AAbstractThread*> visited;
    auto future = hop(home, worker.get(), visited);
    while (!future.hasResult()) {
        loop.iteration();
    }
    EXPECT_NO_THROW(*future);
    EXPECT_EQ(visited, (std::vector<AAbstractThread*> { home, worker.get(), home }));
}

TEST(ThreadSwitch, SameThreadDoesNotSuspend) {
    auto future = []() -> AFuture<int> {
        co_await AThread::current()->switchTo();
        co_return 1;
    }();
    ASSERT_TRUE(future.hasValue());
    EXPECT_EQ(*future, 1);
}

TEST(ThreadSwitch, FollowingAwaitResumesOnTheSwitchedThread) {
    LoopThread worker;
    auto future = [](AAbstractThread* worker) -> AFuture<bool> {
        co_await worker->switchTo();
        co_await (AUI_THREADPOOL { AThread::sleep(std::chrono::milliseconds(1)); });
        co_return AThread::current().get() == worker;
    }(worker.get());
    EXPECT_TRUE(*future);
}

TEST(ThreadSwitch, CancelledCoroutineStopsAfterTheSwitch) {
    LoopThread worker;
    std::atomic_bool reached = false;
    std::atomic_bool release = false;
    // keep the worker busy until the future is dropped
    auto busy = worker->async([&] {
        while (!release) {
            std::this_thread::yield();
        }
    });
    {
        auto future = [](AAbstractThread* worker, std::atomic_bool& reached) -> AFuture<> {
            co_await worker->switchTo();
            reached = true;
        }(worker.get(), reached);
        // the future is dropped before the worker resumes the coroutine, so it unwinds instead of continuing
    }
    release = true;
    auto barrier = worker->async([] {});
    barrier.wait();
    EXPECT_FALSE(reached);
}

#endif
