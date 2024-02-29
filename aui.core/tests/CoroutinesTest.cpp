// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#if AUI_COROUTINES

#include <coroutine>
#include "AUI/Common/AException.h"
#include "AUI/Logging/ALogger.h"
#include <gtest/gtest.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>

static constexpr auto LOG_TAG = "CoroutinesTest";
using namespace std::chrono_literals;



AFuture<int> longTask() {
    return async {
        AThread::sleep(10ms); // long tamssk
        return 228;
    };
}

TEST(Coroutines, CoAwait) {
    auto future = []() -> AFuture<int> {
        ALogger::info(LOG_TAG) << "longTask(): before longTask";
        auto v228 = co_await longTask();
        ALogger::info(LOG_TAG) << "longTask(): after longTask";
        co_return v228 + 322;
    }();
    auto v = *future;
    EXPECT_EQ(v, 228 + 322);
}

AFuture<int> longTaskException() {
    return async -> int {
        AThread::sleep(10ms); // long tamssk
        throw AException("Whoops! Something bad happened");
    };
}

TEST(Coroutines, CoAwaitException) {
    auto future = []() -> AFuture<int> {
        ALogger::info(LOG_TAG) << "longTask(): before longTask";
        auto v228 = co_await longTaskException();
        ALogger::info(LOG_TAG) << "longTask(): after longTask";
        co_return v228 + 322;
    }();
    EXPECT_ANY_THROW(*future);
}

#endif
