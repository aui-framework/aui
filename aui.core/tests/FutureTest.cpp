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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Thread/AAsyncHolder.h>
#include <AUI/Thread/AEventLoop.h>
#include <AUI/Util/kAUI.h>

using namespace std::chrono_literals;

namespace {

/// The deepest cause of an exception chain.
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

template <typename T>
bool isAbandoned(const AFuture<T>& future) {
    try {
        future.get();
    } catch (const AInvocationTargetException& e) {
        try {
            std::rethrow_exception(e.causedBy());
        } catch (const AFutureAbandonedException&) {
            return true;
        } catch (...) {
        }
    }
    return false;
}

}   // namespace

TEST(Future, CreateSuppliesValue) {
    auto future = AFuture<int>::create([](AFuture<int>::Resolver resolver) { resolver.supplyValue(228); });
    EXPECT_EQ(*future, 228);
}

TEST(Future, CreateSuppliesValueLater) {
    AOptional<AFuture<int>::Resolver> resolver;
    auto future = AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); });
    EXPECT_FALSE(future.hasResult());
    resolver->supplyValue(1);
    EXPECT_EQ(*future, 1);
}

TEST(Future, LostResolverAbandonsTheFuture) {
    auto future = AFuture<int>::create([](AFuture<int>::Resolver) {});
    ASSERT_TRUE(future.hasException());
    EXPECT_TRUE(isAbandoned(future));
}

TEST(Future, ResolverLostLaterAbandonsTheFuture) {
    AOptional<AFuture<>::Resolver> resolver;
    auto future = AFuture<>::create([&](AFuture<>::Resolver r) { resolver = std::move(r); });
    bool failed = false;
    future.onException([&](const AException&) { failed = true; });
    resolver.reset();
    EXPECT_TRUE(failed);
    EXPECT_TRUE(isAbandoned(future));
}

TEST(Future, ExceptionInCreateFailsTheFuture) {
    auto future = AFuture<int>::create([](AFuture<int>::Resolver resolver) {
        auto copy = resolver;   // stays alive until unwinding
        throw AException("boom");
    });
    ASSERT_TRUE(future.hasException());
    EXPECT_FALSE(isAbandoned(future));
    try {
        *future;
        FAIL();
    } catch (const AException& e) {
        EXPECT_EQ(rootMessage(e), "boom");
    }
}

TEST(Future, FirstResultWins) {
    auto future = AFuture<int>::create([](AFuture<int>::Resolver resolver) {
        resolver.supplyValue(1);
        resolver.supplyValue(2);
        resolver.supplyException(std::make_exception_ptr(AException("late")));
    });
    EXPECT_EQ(*future, 1);
}

TEST(Future, FromValueAndFromException) {
    EXPECT_EQ(*AFuture<int>::fromValue(5), 5);
    EXPECT_NO_THROW(*AFuture<>::fromValue());
    auto failed = AFuture<int>::fromException(std::make_exception_ptr(AException("nope")));
    EXPECT_TRUE(failed.hasException());
    EXPECT_THROW(*failed, AInvocationTargetException);
}

TEST(Future, DroppingTheLastFutureCancels) {
    AOptional<AFuture<int>::Resolver> resolver;
    bool cancelled = false;
    {
        auto future = AFuture<int>::create([&](AFuture<int>::Resolver r) {
            r.onCancel([&] { cancelled = true; });
            resolver = std::move(r);
        });
        future.onValue([](int) { ADD_FAILURE() << "a cancelled future has called onValue"; });
    }
    EXPECT_TRUE(cancelled);
    EXPECT_TRUE(resolver->isCancelled());
    resolver->supplyValue(1);   // ignored
}

TEST(Future, CopyKeepsTheOperation) {
    AOptional<AFuture<int>::Resolver> resolver;
    auto future = AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); });
    {
        auto copy = future;
    }
    EXPECT_FALSE(resolver->isCancelled());
    resolver->supplyValue(3);
    EXPECT_EQ(*future, 3);
}

TEST(Future, CancelledFutureDoesNotWaitForever) {
    AOptional<AFuture<int>::Resolver> resolver;
    auto future = AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); });
    future.cancel();
    EXPECT_TRUE(future.isCancelled());
    EXPECT_THROW(*future, AInvocationTargetException);
}

TEST(Future, KeepAliveRunsTheOperationToTheEnd) {
    AOptional<AFuture<int>::Resolver> resolver;
    int received = 0;
    AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); })
        .onValue([&](int v) { received = v; })
        .keepAlive();
    EXPECT_FALSE(resolver->isCancelled());
    resolver->supplyValue(42);
    EXPECT_EQ(received, 42);
}

TEST(Future, KeptAliveFutureFailsWhenAbandoned) {
    AOptional<AFuture<int>::Resolver> resolver;
    bool failed = false;
    AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); })
        .onException([&](const AException&) { failed = true; })
        .keepAlive();
    resolver.reset();
    EXPECT_TRUE(failed);
}

TEST(Future, ThreadPoolTaskThrows) {
    auto future = AUI_THREADPOOL -> int { throw AException("from the pool"); };
    try {
        *future;
        FAIL();
    } catch (const AException& e) {
        EXPECT_EQ(rootMessage(e), "from the pool");
    }
}

TEST(Future, InterruptedTaskFails) {
    auto future = AUI_THREADPOOL {
        AThread::current()->interrupt();
        AThread::interruptionPoint();
    };
    // JUST_WAIT: executed by the waiting thread, the task would interrupt it
    EXPECT_THROW(future.get(AFutureWait::JUST_WAIT), AInvocationTargetException);
}

TEST(Future, MapTransformsTheValue) {
    auto mapped = AFuture<int>::fromValue(20).map([](int v) { return v + 1; });
    EXPECT_EQ(*mapped, 21);
    auto toString = AFuture<int>::fromValue(1).map([](int v) { return AString::number(v); });
    EXPECT_EQ(*toString, "1");
}

TEST(Future, MapPassesTheExceptionOn) {
    auto mapped = AFuture<int>::fromException(std::make_exception_ptr(AException("source")))
                      .map([](int v) { return v; });
    try {
        *mapped;
        FAIL();
    } catch (const AException& e) {
        EXPECT_EQ(rootMessage(e), "source");
    }
    auto throwing = AFuture<int>::fromValue(1).map([](int) -> int { throw AException("mapper"); });
    EXPECT_THROW(*throwing, AInvocationTargetException);
}

TEST(Future, MapFlattensAFuture) {
    AOptional<AFuture<AString>::Resolver> second;
    auto chained = AFuture<int>::fromValue(1).map([&](int v) {
        return AFuture<AString>::create([&](AFuture<AString>::Resolver r) { second = std::move(r); });
    });
    static_assert(std::is_same_v<decltype(chained), AFuture<AString>>);
    EXPECT_FALSE(chained.hasResult());
    second->supplyValue("two");
    EXPECT_EQ(*chained, "two");
}

TEST(Future, MapFlattenedExceptionKeepsTheCause) {
    auto chained = AFuture<int>::fromValue(1).map([](int) {
        return AFuture<int>::fromException(std::make_exception_ptr(AException("second step")));
    });
    try {
        *chained;
        FAIL();
    } catch (const AException& e) {
        EXPECT_EQ(rootMessage(e), "second step");
    }
}

TEST(Future, DroppingMappedFutureCancelsTheFlattenedStep) {
    AOptional<AFuture<int>::Resolver> second;
    {
        auto chained = AFuture<>::fromValue().map([&] {
            return AFuture<int>::create([&](AFuture<int>::Resolver r) { second = std::move(r); });
        });
        EXPECT_FALSE(second->isCancelled());
    }
    EXPECT_TRUE(second->isCancelled());
}

TEST(Future, DroppingMappedFutureCancelsTheSource) {
    AOptional<AFuture<int>::Resolver> resolver;
    {
        auto mapped = AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); }).map([](int v) {
            return v;
        });
        EXPECT_FALSE(resolver->isCancelled());
    }
    EXPECT_TRUE(resolver->isCancelled());
}

TEST(Future, AllCollectsValuesInOrder) {
    AOptional<AFuture<int>::Resolver> first;
    AOptional<AFuture<AString>::Resolver> second;
    auto all = AFuture<>::all(
        AFuture<int>::create([&](AFuture<int>::Resolver r) { first = std::move(r); }),
        AFuture<AString>::create([&](AFuture<AString>::Resolver r) { second = std::move(r); }),
        AFuture<>::fromValue());
    second->supplyValue("two");
    EXPECT_FALSE(all.hasResult());
    first->supplyValue(1);
    ASSERT_TRUE(all.hasValue());
    auto [a, b, c] = *all;
    EXPECT_EQ(a, 1);
    EXPECT_EQ(b, "two");
    EXPECT_EQ(c, std::monostate {});
}

TEST(Future, AllFailsFastAndCancelsTheRest) {
    AOptional<AFuture<int>::Resolver> slow;
    AOptional<AFuture<int>::Resolver> failing;
    auto all = AFuture<>::all(
        AFuture<int>::create([&](AFuture<int>::Resolver r) { slow = std::move(r); }),
        AFuture<int>::create([&](AFuture<int>::Resolver r) { failing = std::move(r); }));
    failing->supplyException(std::make_exception_ptr(AException("failed")));
    ASSERT_TRUE(all.hasException());
    EXPECT_TRUE(slow->isCancelled());
    try {
        *all;
        FAIL();
    } catch (const AException& e) {
        EXPECT_EQ(rootMessage(e), "failed");
    }
}

TEST(Future, AllOfNothing) {
    auto all = AFuture<>::all();
    EXPECT_TRUE(all.hasValue());
}

TEST(Future, DroppingAllCancelsItsFutures) {
    AOptional<AFuture<int>::Resolver> a;
    AOptional<AFuture<>::Resolver> b;
    {
        auto all = AFuture<>::all(
            AFuture<int>::create([&](AFuture<int>::Resolver r) { a = std::move(r); }),
            AFuture<>::create([&](AFuture<>::Resolver r) { b = std::move(r); }));
    }
    EXPECT_TRUE(a->isCancelled());
    EXPECT_TRUE(b->isCancelled());
}

TEST(Future, AnyTakesTheFirstValue) {
    AOptional<AFuture<int>::Resolver> first;
    AOptional<AFuture<AString>::Resolver> second;
    auto any = AFuture<>::any(
        AFuture<int>::create([&](AFuture<int>::Resolver r) { first = std::move(r); }),
        AFuture<AString>::create([&](AFuture<AString>::Resolver r) { second = std::move(r); }));
    second->supplyValue("second");
    ASSERT_TRUE(any.hasValue());
    EXPECT_EQ(any->index(), 1);
    EXPECT_EQ(std::get<1>(*any), "second");
    EXPECT_TRUE(first->isCancelled());
}

TEST(Future, AnyTakesTheFirstException) {
    AOptional<AFuture<int>::Resolver> first;
    auto any = AFuture<>::any(
        AFuture<int>::create([&](AFuture<int>::Resolver r) { first = std::move(r); }),
        AFuture<int>::fromException(std::make_exception_ptr(AException("first to fail"))));
    ASSERT_TRUE(any.hasException());
    EXPECT_TRUE(first->isCancelled());
}

TEST(Future, AnyOfVoid) {
    auto any = AFuture<>::any(AFuture<>::fromValue(), AFuture<int>::fromValue(1));
    EXPECT_EQ(any->index(), 0);
}

TEST(Future, WithTimeoutPassesTheValue) {
    auto future = AUI_THREADPOOL { return 1; };
    EXPECT_EQ(*future.withTimeout(5s), 1);
}

TEST(Future, WithTimeoutFails) {
    AOptional<AFuture<int>::Resolver> resolver;
    auto future = AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); }).withTimeout(50ms);
    try {
        *future;
        FAIL();
    } catch (const AInvocationTargetException& e) {
        EXPECT_THAT(rootMessage(e), testing::HasSubstr("timed out"));
    }
    EXPECT_TRUE(resolver->isCancelled());
}

TEST(Future, WithTimeoutVoid) {
    auto future = AThread::asyncSleep(10ms).withTimeout(5s);
    EXPECT_NO_THROW(*future);
}

#if AUI_COROUTINES

namespace {
AFuture<int> awaitAll(AFuture<int> a, AFuture<int> b) {
    auto [x, y] = co_await AFuture<>::all(std::move(a), std::move(b));
    co_return x + y;
}

AFuture<int> neverResumed(AFuture<int> never) { co_return co_await never; }
}   // namespace

TEST(Future, CoAwaitAll) {
    AEventLoop loop;
    IEventLoop::Handle h(&loop);
    auto sum = awaitAll(AUI_THREADPOOL { return 1; }, AUI_THREADPOOL { return 2; });
    while (!sum.hasResult()) {
        loop.iteration();
    }
    EXPECT_EQ(*sum, 3);
}

TEST(Future, CoroutineWaitingForAnAbandonedFutureFails) {
    AEventLoop loop;
    IEventLoop::Handle h(&loop);
    AOptional<AFuture<int>::Resolver> resolver;
    auto result = neverResumed(AFuture<int>::create([&](AFuture<int>::Resolver r) { resolver = std::move(r); }));
    resolver.reset();
    while (!result.hasResult()) {
        loop.iteration();
    }
    try {
        *result;
        FAIL();
    } catch (const AInvocationTargetException& e) {
        EXPECT_EQ(rootMessage(e), AFutureAbandonedException().getMessage());
    }
}

#endif
