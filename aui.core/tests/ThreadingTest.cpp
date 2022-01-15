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

//
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Common/SharedPtr.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>
#include <random>
#include <ctime>


TEST(Threading, Async) {
    auto someInt = _new<std::atomic_int>(0);
    for (int i = 0; i < 100; ++i)
        async{
                (*someInt) += 1;
        };

    AThread::sleep(1000);

    ASSERT_EQ(*someInt, 100);
}

TEST(Threading, SleepInterruption) {
    bool called = false;
    auto future = asyncX [&] {
        called = true;
        AThread::sleep(1000);
        ADD_FAILURE() << "this line should not have reached";
        return 0;
    };
    AThread::sleep(500);
    future.cancel();
    AThread::sleep(1000);
    ASSERT_TRUE(called) << "lambda has not called either";

}
TEST(Threading, ConditionVariableInterruption) {
    bool called = false;
    auto future = asyncX [&] {
        called = true;
        AConditionVariable cv;
        AMutex mutex;
        std::unique_lock lock(mutex);
        cv.wait(lock);
        ADD_FAILURE() << "this line should not have reached";
        return 0;
    };
    AThread::sleep(500);
    future.cancel();
    AThread::sleep(500);
    ASSERT_TRUE(called) << "lambda has not called either";
}

TEST(Threading, Future1) {
    ADeque<AFuture<double>> taskList;
    auto time = Util::measureTimeInMillis([&]() {
        repeat(1000)
        {
            taskList << async{
                    double i = 2.0;
                    repeat(1000)
                    {
                        i *= 2;
                    }
                    return i;
            };
        }

        for (auto& f : taskList) *f;

        printf("Ok, result: %f\n", *taskList.first());
    });

    printf("Finished in %llu ms\n", time);
    ASSERT_EQ(*taskList.first(),
                      21430172143725346418968500981200036211228096234110672148875007767407021022498722449863967576313917162551893458351062936503742905713846280871969155149397149607869135549648461970842149210124742283755908364306092949967163882534797535118331087892154125829142392955373084335320859663305248773674411336138752.000000);
    ASSERT_TRUE(time < 1000);
}

TEST(Threading, Future2) {
    auto b = _new<bool>(false);
    async{
            AThread::sleep(1000);
            *b = true;
    };
    ASSERT_TRUE(!*b);
    AThread::sleep(2000);
    ASSERT_TRUE(*b);
}

TEST(Threading, Future3) {
    auto b = _new<bool>(false);
    auto v8 = async{
            AThread::sleep(1000);
            *b = true;
            return 8;
    };
    auto v1231 = async{
            AThread::sleep(1000);
            *b = true;
            return 1231.f;
    };
    ASSERT_TRUE(!*b);
    AThread::sleep(2000);
    ASSERT_TRUE(*b);

    ASSERT_EQ(*v8, 8);
    ASSERT_EQ(*v1231, 1231.f);
}

TEST(Threading, PararellVoid) {
    for (int i = 0; i < 1000; ++i) {
        AVector<int> ints;
        for (int j = 0; j < i; ++j) {
            ints.push_back(j);
        }
        AThreadPool::global().parallel(ints.begin(),
                                       ints.end(),
                                       [](AVector<int>::iterator begin, AVector<int>::iterator end) {
            for (auto it = begin; it != end; ++it) {
                *it += 2;
            }
            return 0;
        }).waitForAll();

        for (int j = 0; j < i; ++j) {
            if (ints[j] != j + 2) ADD_FAILURE() << "invalid result";
        }
    }
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
        if (accumulator != 5 * i) ADD_FAILURE() << "invalid result";
    }
}

TEST(Threading, FutureCancellationBeforeExecution) {
    AThreadPool localThreadPool(1);
    size_t foreignLambdaCallCount = 0;
    auto foreignLambda = [&] {
        try {
            AThread::sleep(500);
        } catch (const AThread::AInterrupted& e) {
            ADD_FAILURE() << "interrupted exception thrown in a foreign lambda";
        }
        foreignLambdaCallCount += 1;
    };
    localThreadPool * foreignLambda;
    {
        auto future = localThreadPool * [&] {
            ADD_FAILURE() << "lambda has called";
            return 0;
        };
        localThreadPool * foreignLambda;

        AThread::sleep(250);
    }
    AThread::sleep(1000);
    ASSERT_EQ(foreignLambdaCallCount, 2);
}

TEST(Threading, FutureCancellationWhileExecution) {
    bool called = false;
    {
        auto future = asyncX [&] {
            called = true;
            // hard work
            AThread::sleep(1000);
            ADD_FAILURE() << "this line should not have reached";
            return 0;
        };
        AThread::sleep(500);
        ASSERT_TRUE(called) << "lambda has not called either";
    }
    AThread::sleep(2000);
}

TEST(Threading, FutureCancellationAfterExecution) {
    AThreadPool localThreadPool(1);
    bool foreignLambdaCalled = false;
    {
        auto future = localThreadPool * [&] {
            return 0;
        };
        localThreadPool * [&] {
            foreignLambdaCalled = true;
        };
        AThread::sleep(250);
    }
    AThread::sleep(250);
    ASSERT_TRUE(foreignLambdaCalled) << "foreign lambda has not called";
}


TEST(Threading, FutureOnDone) {
    bool called = false;
    {
        auto future = async {
            return 322;
        };
        future.onSuccess([&](int i) {
            ASSERT_EQ(i, 322);
            called = true;
        });
        // check that cancellation does not triggers here
    }
    AThread::sleep(500);
    ASSERT_TRUE(called) << "onSuccess callback has not called";
}
/*
TEST(Threading, Fence) {
    std::default_random_engine e(std::time(nullptr));
    repeat(10) {
        std::atomic_int test = 0;
        std::mutex m;

        AThreadPool::global().fence([&]() {
            repeat(1000) {
                asyncX [&]() {
                    int s;
                    {
                        std::unique_lock lock(m);
                        s = std::uniform_int_distribution(5, 100)(e);
                    }
                    AThread::sleep(s);
                    ++test;
                };
            }

            EXPECT_TRUE_LE(test, 1000);
        });
        ASSERT_EQ(test, 1000);
    }
}*/
