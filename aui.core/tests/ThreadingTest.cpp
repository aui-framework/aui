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

//
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>
#include <chrono>
#include <random>
#include <ctime>
#include "AUI/Common/ATimer.h"
#include "AUI/Thread/AThread.h"
#include "AUI/Traits/parallel.h"
#include "AUI/Thread/AAsyncHolder.h"
#include "AUI/Util/ARaiiHelper.h"

using namespace std::chrono_literals;

TEST(Threading, Async) {
    auto someInt = _new<std::atomic_int>(0);
    AFutureSet<> f;
    for (int i = 0; i < 100; ++i) {
        f << AUI_THREADPOOL {
            (*someInt) += 1;
        };
    }
    f.waitForAll();

    ASSERT_EQ(*someInt, 100);
}

TEST(Threading, SleepInterruption) {
    bool called = false;
    auto future = AUI_THREADPOOL_X [&] {
        called = true;
        AThread::sleep(1000ms);
        ADD_FAILURE() << "this line should not have reached";
        return 0;
    };
    AThread::sleep(500ms);
    future.cancel();
    AThread::sleep(1000ms);
    ASSERT_TRUE(called) << "lambda has not called either";

}
TEST(Threading, ConditionVariableInterruption) {
    bool called = false;
    auto future = AUI_THREADPOOL_X [&] {
        called = true;
        AConditionVariable cv;
        AMutex mutex;
        std::unique_lock lock(mutex);
        cv.wait(lock);
        ADD_FAILURE() << "this line should not have reached";
        return 0;
    };
    AThread::sleep(500ms);
    future.cancel();
    AThread::sleep(500ms);
    ASSERT_TRUE(called) << "lambda has not called either";
}

TEST(Threading, Future1) {
    ADeque<AFuture<double>> taskList;
    auto time = util::measureExecutionTime<std::chrono::milliseconds>([&]() {
        AUI_REPEAT(1000)
        {
            taskList << AUI_THREADPOOL{
                    double i = 2.0;
                    AUI_REPEAT(1000)
                    {
                        i *= 2;
                    }
                    return i;
            };
        }

        for (auto& f : taskList) *f;
    }).count();
    ASSERT_EQ(*taskList.first(),
                      21430172143725346418968500981200036211228096234110672148875007767407021022498722449863967576313917162551893458351062936503742905713846280871969155149397149607869135549648461970842149210124742283755908364306092949967163882534797535118331087892154125829142392955373084335320859663305248773674411336138752.000000);
    ASSERT_TRUE(time < 1000);
}

TEST(Threading, Future2) {
    auto b = _new<bool>(false);
    auto f = AUI_THREADPOOL{
            AThread::sleep(1000ms);
            *b = true;
    };
    ASSERT_TRUE(!*b);
    AThread::sleep(2000ms);
    ASSERT_TRUE(*b);
}

TEST(Threading, Future3) {
    auto b = _new<bool>(false);
    auto v8 = AUI_THREADPOOL{
            AThread::sleep(1000ms);
            *b = true;
            return 8;
    };
    auto v1231 = AUI_THREADPOOL{
            AThread::sleep(1000ms);
            *b = true;
            return 1231.f;
    };
    ASSERT_TRUE(!*b);
    AThread::sleep(2000ms);
    ASSERT_TRUE(*b);

    ASSERT_EQ(*v8, 8);
    ASSERT_EQ(*v1231, 1231.f);
}

TEST(Threading, ParallelVoid) {
    bool watchdogTrigger = false;
    auto watchdog = _new<AThread>([&] {
        while (!AThread::current()->isInterrupted()) {
            AThread::sleep(5000ms);
            if (watchdogTrigger) {
                ADD_FAILURE() << "deadlock";
                exit(-1);
            } else {
                watchdogTrigger = true;
            }
        }
    });
    watchdog->start();
    for (int i = 0; i < 1000; ++i) {
        AVector<int> ints;
        for (int j = 0; j < i; ++j) {
            ints.push_back(j);
        }
        AThreadPool tp(2);
        tp.parallel(ints.begin(),
                    ints.end(),
                    [](AVector<int>::iterator begin, AVector<int>::iterator end) {
                        for (auto it = begin; it != end; ++it) {
                            *it += 2;
                        }
                        return 0;
                    }).waitForAll();

        for (int j = 0; j < i; ++j) {
            if (ints[j] != j + 2) ADD_FAILURE() << "invalid supplyValue";
        }
        watchdogTrigger = false;
    }
    watchdog->interrupt();
    watchdog->join();
}

TEST(Threading, PararellWithResult) {
    for (int i = 0; i < 1000; ++i) {
        AVector<int> ints;
        for (int j = 0; j < i; ++j) {
            ints.push_back(5);
        }
        auto result = AThreadPool::global().parallel(ints.begin(),
                                       ints.end(),
                                       [](AVector<int>::iterator begin, AVector<int>::iterator end) {
            int r = 0;
            for (auto it = begin; it != end; ++it) {
                r += *it;
            }
            return r;
        });
        int accumulator = 0;
        for (auto& v : result) {
            accumulator += *v;
        }
        auto expected = i * 5;
        if (accumulator != expected) ADD_FAILURE() << "invalid supplyValue: iteration " << i << ", expected = " << expected << ", actual = " << accumulator;
    }
}

TEST(Threading, FutureCancellationBeforeExecution) {
    AThreadPool localThreadPool(1);
    size_t foreignLambdaCallCount = 0;
    auto foreignLambda = [&] {
        try {
            AThread::sleep(500ms);
        } catch (const AThread::Interrupted& e) {
            ADD_FAILURE() << "interrupted exception thrown in a foreign lambda";
        }
        foreignLambdaCallCount += 1;
    };
    auto exec = localThreadPool * foreignLambda;
    {
        AFuture<> future;
        {
            auto future1 = localThreadPool * [&] {
                ADD_FAILURE() << "lambda has called";
            };
            future = localThreadPool * foreignLambda;
        }
        future.wait();
        AThread::sleep(250ms);
    }
    AThread::sleep(500ms);
    ASSERT_EQ(foreignLambdaCallCount, 2);
    exec.wait();
}

TEST(Threading, FutureCancellationWhileExecution) {
    bool called = false;
    {
        auto future = AUI_THREADPOOL_X [&] {
            called = true;
            // hard work
            AThread::sleep(1000ms);
            ADD_FAILURE() << "this line should not have reached";
        };
        AThread::sleep(500ms);
        ASSERT_TRUE(called) << "lambda has not called either";
    }
    AThread::sleep(2000ms);
}

TEST(Threading, FutureCancellationAfterExecution) {
    AThreadPool localThreadPool(1);
    bool foreignLambdaCalled = false;
    {
        auto future1 = localThreadPool * [&] {
            return 0;
        };
        auto future2 = localThreadPool * [&] {
            foreignLambdaCalled = true;
        };
        AThread::sleep(250ms);
    }
    AThread::sleep(250ms);
    ASSERT_TRUE(foreignLambdaCalled) << "foreign lambda has not called";
}


TEST(Threading, FutureInterruptionCascade) {
    {
        static bool interrupted = false;
        {
            auto t = AUI_THREADPOOL_X [&] {
                auto items = AVector<int>::generate(10000, [](std::size_t i) { return i; });
                auto tasks = AUI_PARALLEL_MP(items) {
                    for (auto it = begin; it != end; ++it) {
                        EXPECT_FALSE(interrupted);
                        AThread::sleep(100ms);
                    }
                };
                tasks.waitForAll();
            };
            AThread::sleep(500ms);
            t.cancel();
        }
        interrupted = true;
    }
}


TEST(Threading, FutureOnDone1) {

    AUI_REPEAT(100) {
        AThreadPool localThreadPool(1);
        localThreadPool.run([&] {
            AThread::sleep(std::chrono::milliseconds(repeatStubIndex)); // long task
        });


        bool called = false;
        {
            auto future = localThreadPool * [] {
                return 322;
            };
            future.onSuccess([&](int i) {
                ASSERT_EQ(i, 322);
                called = true;
            });
            // check that cancellation does not triggers here
            future.wait(AFutureWait::JUST_WAIT);

            AThread::sleep(10ms); // extra delay for the callback to be called
        }
        ASSERT_TRUE(called) << "onSuccess callback has not called (iteration " << repeatStubIndex << ")";
    }
}
TEST(Threading, FutureOnDone2) {

    AUI_REPEAT(100) {
        AThreadPool localThreadPool(1);
        localThreadPool.run([&] {
        });


        bool called = false;
        {
            auto future = localThreadPool * [] {
                return 322;
            };
            AThread::sleep(std::chrono::milliseconds(repeatStubIndex)); // long task
            future.onSuccess([&](int i) {
                ASSERT_EQ(i, 322);
                called = true;
            });
            // check that cancellation does not triggers here
            future.wait(AFutureWait::JUST_WAIT);

            AThread::sleep(10ms); // extra delay for the callback to be called
        }
        ASSERT_TRUE(called) << "onSuccess callback has not called (iteration " << repeatStubIndex << ")";
    }
}

TEST(Threading, FutureOnSuccess) {
    AThreadPool localThreadPool(1);
    localThreadPool.run([] {
        AThread::sleep(1s); // long tamssk
    });


    bool destructorCalled = false;                                                             // used to check future
    std::function<void()> destructorCallback = [&destructorCalled] {                           // destruction
        destructorCalled = true;                                                               //
    };                                                                                         //

    AAsyncHolder holder;
    bool called = false;
    {
        auto future = localThreadPool * [] {
            return 322;
        };
        holder << future.onSuccess([&, destructorCallback = std::move(destructorCallback)](int i) {
            ARaiiHelper raii = std::move(destructorCallback);
            ASSERT_EQ(i, 322);
            called = true;
        });
    }
    AThread::sleep(2000ms);
    ASSERT_TRUE(called) << "onSuccess callback has not called";
    ASSERT_TRUE(destructorCalled) << "AFuture internal logic is not destructed";
    ASSERT_TRUE(holder.size() == 0) << "holder is not empty";
}

TEST(Threading, FutureExecuteOnCallingThread) {
    AThreadPool localThreadPool(1);
    localThreadPool.run([] {
        AThread::sleep(1s); // long tamssk
    });

    auto callerThreadId = std::this_thread::get_id();
    bool called = false;
    auto future = localThreadPool * [&called, callerThreadId] {
        called = true;
        EXPECT_EQ(std::this_thread::get_id(), callerThreadId);
    };
    future.wait();
    ASSERT_TRUE(called) << "callback has not called";
}

TEST(Threading, FutureSelfLock1) {
    {
        AThreadPool localThreadPool(1);
        AFuture<int> future;
        future = localThreadPool * [&future] {
            AThread::sleep(1s);
            *future;
            return 1;
        };
        EXPECT_ANY_THROW(*future);
    }
    {
        AThreadPool localThreadPool(1);
        AFuture<> future;
        future = localThreadPool * [&future] {
            AThread::sleep(1s);
            *future;
        };
        EXPECT_ANY_THROW(*future);
    }
}

TEST(Threading, FutureSelfLock2) {
    {
        AThreadPool localThreadPool(1);
        AFuture<int> future;
        future = localThreadPool * [&future] {
            AThread::sleep(1s);
            future.wait();
            return 1;
        };
        EXPECT_ANY_THROW(*future);
    }
    {
        AThreadPool localThreadPool(1);
        AFuture<> future;
        future = localThreadPool * [&future] {
            AThread::sleep(1s);
            future.wait();
        };
        EXPECT_ANY_THROW(*future);
    }
}

TEST(Threading, AsyncHolder) {

    bool holderDestroyed = false;

    AThreadPool localThreadPool(1);
    {
        AAsyncHolder holder;
        holder << localThreadPool * [&] {
            AThread::sleep(500ms);
            ASSERT_FALSE(holderDestroyed);
        };
        holder << localThreadPool * [&] {
            AThread::sleep(500ms);
            ASSERT_FALSE(holderDestroyed);
        };
        AThread::sleep(700ms);
        EXPECT_EQ(holder.size(), 1);
    }
    holderDestroyed = true;
}
