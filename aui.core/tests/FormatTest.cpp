/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include <AUI/Traits/algorithms.h>
#include <AUI/Common/AVector.h>
#include <AUI/Traits/strings.h>

TEST(Format, String) {
    EXPECT_EQ("test {}"_format("azaza"), "test azaza");
}

TEST(Format, Int) {
    EXPECT_EQ("test {}"_format(1), "test 1");
    EXPECT_EQ("test {}"_format(12), "test 12");
    EXPECT_EQ("test {}"_format(123), "test 123");
    EXPECT_EQ("test {}"_format(123456780), "test 123456780");
    EXPECT_EQ("test {}"_format(1234567890), "test 1234567890");
}

TEST(Format, IntNegative) {
    EXPECT_EQ("test {}"_format(-1), "test -1");
    EXPECT_EQ("test {}"_format(-12), "test -12");
    EXPECT_EQ("test {}"_format(-123), "test -123");
    EXPECT_EQ("test {}"_format(-123456780), "test -123456780");
    EXPECT_EQ("test {}"_format(-1234567890), "test -1234567890");
}
TEST(Format, Float) {
    EXPECT_EQ("test {:.2}"_format(1.2f), "test 1.2");
}
