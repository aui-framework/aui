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


#include "AUI/Common/AException.h"
#include "AUI/Thread/AThreadPool.h"
#include <gtest/gtest.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>
#include <chrono>

static constexpr auto LOG_TAG = "StackfulCoroutinesTest";
using namespace std::chrono;
using namespace std::chrono_literals;



AFuture<int> longTask(high_resolution_clock::time_point* returnTimePoint, milliseconds duration) {
    return AUI_THREADPOOL {
        AThread::sleep(duration); // long tamssk
        *returnTimePoint = high_resolution_clock::now();
        return 228;
    };
}

TEST(StackfulCoroutines, CoAwait) {
    AThreadPool localThreadPool(1);
    int executionOrder = 0;


    // Execution of the first function should not block the execution of the second function; ultimately, the second
    // function should finish faster than the first function.

    auto future1 = localThreadPool * [&]() {
        high_resolution_clock::time_point returnTimePoint;
        auto f = longTask(&returnTimePoint, 1s);
        f.wait(AFutureWait::ALLOW_STACKFUL_COROUTINES);
        EXPECT_EQ(*f, 228);
        EXPECT_EQ(executionOrder++, 1);
    };

    auto future2 = localThreadPool * [&]() {
        high_resolution_clock::time_point returnTimePoint;
        auto f = longTask(&returnTimePoint, 1ms);
        f.wait(AFutureWait::ALLOW_STACKFUL_COROUTINES);
        EXPECT_EQ(*f, 228);
        EXPECT_EQ(executionOrder++, 0);
    };
    future1.wait(AFutureWait::JUST_WAIT);
    future2.wait(AFutureWait::JUST_WAIT);
}

AFuture<int> longTaskException() {
    return AUI_THREADPOOL -> int {
        AThread::sleep(10ms); // long tamssk
        throw AException("Whoops! Something bad happened");
    };
}
