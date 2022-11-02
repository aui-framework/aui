/*
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
