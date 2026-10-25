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
#include "AUI/UITest.h"
#include "AUI/Remote/Agent/JsonRpc.h"
#include "AUI/Remote/Agent/RefTable.h"
#include "AUI/Remote/Agent/UiThread.h"

using namespace aui::remote;

class RefTableTest : public testing::UITest {};

TEST_F(RefTableTest, ResolvesWhatWasAdded) {
    RefTable table;
    auto view = _new<AView>();
    auto ref = table.add(view);
    EXPECT_EQ(table.resolve(ref), view);
}

TEST_F(RefTableTest, SameViewSameRefWithinGeneration) {
    RefTable table;
    auto a = _new<AView>();
    auto b = _new<AView>();
    EXPECT_EQ(table.add(a), table.add(a));
    EXPECT_NE(table.add(a), table.add(b));
}

TEST_F(RefTableTest, NewGenerationGivesNewRefsButOldOnesStillResolve) {
    RefTable table;
    auto view = _new<AView>();
    auto first = table.add(view);
    table.newGeneration();
    auto second = table.add(view);
    EXPECT_NE(first, second);
    EXPECT_EQ(table.resolve(first), view);
    EXPECT_EQ(table.resolve(second), view);
}

TEST_F(RefTableTest, RefDoesNotKeepViewAlive) {
    RefTable table;
    AString ref;
    _weak<AView> weak;
    {
        auto view = _new<AView>();
        weak = view;
        ref = table.add(view);
    }
    EXPECT_TRUE(weak.expired());
    try {
        (void) table.resolve(ref);
        FAIL() << "expected STALE_REF";
    } catch (const RpcException& e) {
        EXPECT_EQ(e.code(), rpc_code::STALE_REF);
        EXPECT_FALSE(e.hint().empty());
    }
}

TEST_F(RefTableTest, MalformedRefsAreStale) {
    RefTable table;
    for (const char* ref : { "", "x", "v", "v1", "v1:", "v:1", "va:1", "v1:b", "1:2", "v1:2:3", "v-1:2", "v1:2 " }) {
        try {
            (void) table.resolve(ref);
            FAIL() << "expected STALE_REF for '" << ref << "'";
        } catch (const RpcException& e) {
            EXPECT_EQ(e.code(), rpc_code::STALE_REF) << ref;
        }
    }
}

TEST_F(RefTableTest, UnknownRefIsStale) {
    RefTable table;
    auto view = _new<AView>();
    (void) table.add(view);
    EXPECT_THROW((void) table.resolve("v1:999"), RpcException);
    EXPECT_THROW((void) table.resolve("v7:1"), RpcException);   // right entry, wrong generation
}

TEST_F(RefTableTest, ForgetsOldestBeyondCapacity) {
    RefTable table(3);
    std::vector<_<AView>> views;
    std::vector<AString> refs;
    for (int i = 0; i < 5; ++i) {
        views.push_back(_new<AView>());
        refs.push_back(table.add(views.back()));
    }
    EXPECT_EQ(table.size(), 3u);
    EXPECT_THROW((void) table.resolve(refs[0]), RpcException);
    EXPECT_THROW((void) table.resolve(refs[1]), RpcException);
    EXPECT_EQ(table.resolve(refs[4]), views[4]);
}

TEST(UiThread, RunsOnTheMainThreadAndReturnsTheResult) {
    auto future = onUiThread([] { return AThread::current() == AThread::main() ? 42 : -1; });
    EXPECT_FALSE(future.hasResult());   // never inline
    AThread::processMessages();
    ASSERT_TRUE(future.hasResult());
    EXPECT_EQ(*future, 42);
}

TEST(UiThread, ExceptionsEndUpInTheFuture) {
    auto future = onUiThread([]() -> int { throw AException("nope"); });
    AThread::processMessages();
    ASSERT_TRUE(future.hasResult());
    EXPECT_ANY_THROW((void) *future);
}

TEST(UiThread, VoidFunctions) {
    int calls = 0;
    auto future = onUiThread([&] { ++calls; });
    AThread::processMessages();
    EXPECT_TRUE(future.hasResult());
    EXPECT_EQ(calls, 1);
}
