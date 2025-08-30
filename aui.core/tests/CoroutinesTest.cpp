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
#include <gtest/gtest.h>
#include <AUI/Thread/AFuture.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Util/Util.h>

static constexpr auto LOG_TAG = "CoroutinesTest";
using namespace std::chrono_literals;



AFuture<int> longTask() {
    return AUI_THREADPOOL {
        AThread::sleep(10ms); // long tamssk
        return 228;
    };
}

TEST(Coroutines, CoAwait) {
    auto future = []() -> AFuture<int> {
        auto v228 = co_await longTask();
        co_return v228 + 322;
    }();
    auto v = *future;
    EXPECT_EQ(v, 228 + 322);
}

AFuture<int> longTaskException() {
    return AUI_THREADPOOL -> int {
        AThread::sleep(10ms); // long tamssk
        throw AException("Whoops! Something bad happened");
    };
}

TEST(Coroutines, CoAwaitException) {
    auto future = []() -> AFuture<int> {
        auto v228 = co_await longTaskException();
        co_return v228 + 322;
    }();
    EXPECT_ANY_THROW(*future);
}

#endif
