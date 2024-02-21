// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.


#include "AUI/Common/AException.h"
#include "AUI/Logging/ALogger.h"
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
    return async {
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
        ALogger::info(LOG_TAG) << "AFuture time between return and value acquired: " << duration_cast<microseconds>(high_resolution_clock::now() - returnTimePoint).count() << "us";
        EXPECT_EQ(executionOrder++, 1);
    };

    auto future2 = localThreadPool * [&]() {
        high_resolution_clock::time_point returnTimePoint;
        auto f = longTask(&returnTimePoint, 1ms);
        f.wait(AFutureWait::ALLOW_STACKFUL_COROUTINES);
        EXPECT_EQ(*f, 228);
        ALogger::info(LOG_TAG) << "AFuture time between return and value acquired: " << duration_cast<microseconds>(high_resolution_clock::now() - returnTimePoint).count() << "us";
        EXPECT_EQ(executionOrder++, 0);
    };
    future1.wait(AFutureWait::JUST_WAIT);
    future2.wait(AFutureWait::JUST_WAIT);
}

AFuture<int> longTaskException() {
    return async -> int {
        AThread::sleep(10ms); // long tamssk
        throw AException("Whoops! Something bad happened");
    };
}
