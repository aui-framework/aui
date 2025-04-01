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
// Created by alex2 on 31.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Traits/values.h>

TEST(Lazy, Unused) {
    aui::lazy<int> unused = [] {
        EXPECT_TRUE(false) << "initializer function called";
        return 0;
    };
}
TEST(Lazy, Used) {
    bool called = false;
    aui::lazy<int> used = [&] {
        EXPECT_FALSE(called) << "initializer function called twice";
        called = true;
        return 123;
    };
    ASSERT_FALSE(called);
    ASSERT_EQ(int(used), 123);
    ASSERT_TRUE(called);
    ASSERT_EQ(*used, 123);
    ASSERT_EQ(used, 123);

    ASSERT_EQ(int((const aui::lazy<int>)used), 123);
    ASSERT_EQ(*((const aui::lazy<int>)used), 123);
    ASSERT_EQ(((const aui::lazy<int>)used), 123);

    ASSERT_TRUE(called);
}
