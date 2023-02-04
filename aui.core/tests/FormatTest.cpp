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
