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
using namespace std::chrono_literals;



AFuture<int> longTask(std::chrono::milliseconds duration) {
    return async {
        AThread::sleep(duration); // long tamssk
        return 228;
    };
}

TEST(StackfulCoroutines, CoAwait) {
    AThreadPool localThreadPool(1);
    int executionOrder = 0;


    // Execution of the first function should not block the execution of the second function; ultimately, the second
    // function should finish faster than the first function.

    auto future1 = localThreadPool * [&]() {
        EXPECT_EQ(*longTask(1s), 228);
        EXPECT_EQ(executionOrder++, 1);
    };

    auto future2 = localThreadPool * [&]() {
        EXPECT_EQ(*longTask(1ms), 228);
        EXPECT_EQ(executionOrder++, 0);
    };
    future1.wait(AFutureWait::ASYNC_ONLY);
    future2.wait(AFutureWait::ASYNC_ONLY);
}


