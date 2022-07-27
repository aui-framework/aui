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
// Created by alex2 on 30.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Common/AString.h>


TEST(Strings, ToInt) {
    EXPECT_EQ("123"_as.toInt(), 123);
    EXPECT_EQ("0"_as.toInt(), 0);
    EXPECT_EQ("-251"_as.toInt(), -251);
    EXPECT_EQ("123abs"_as.toInt(), std::nullopt);
    EXPECT_EQ("1a23"_as.toInt(), std::nullopt);
    EXPECT_EQ("a123"_as.toInt(), std::nullopt);
    EXPECT_EQ("0x"_as.toInt(), 0);
    EXPECT_EQ("0x123"_as.toInt(), 0x123);
    EXPECT_EQ("0xabcdef"_as.toInt(), 0xabcdef);
    EXPECT_EQ("0xabcdef123456"_as.toInt(), std::nullopt); // overflow
    EXPECT_EQ("123456214214"_as.toInt(), std::nullopt); // overflow
}

TEST(Strings, ToUInt) {
    EXPECT_EQ("123"_as.toUInt(), 123);
    EXPECT_EQ("123abs"_as.toUInt(), std::nullopt);
    EXPECT_EQ("1a23"_as.toUInt(), std::nullopt);
    EXPECT_EQ("a123"_as.toUInt(), std::nullopt);
}

TEST(Strings, ToFloat) {
    constexpr float ABS_ERROR = 0.01;

    EXPECT_NEAR("123"_as.toFloat().value_or(0), 123, ABS_ERROR);
    EXPECT_NEAR("123.456"_as.toFloat().value_or(0), 123.456, ABS_ERROR);
    EXPECT_NEAR("-123.456"_as.toFloat().value_or(0), -123.456, ABS_ERROR);
    EXPECT_NEAR("-0.0"_as.toFloat().value_or(99999), 0, ABS_ERROR);
    EXPECT_EQ("123abs"_as.toUInt(), std::nullopt);
    EXPECT_EQ("1a23"_as.toUInt(), std::nullopt);
    EXPECT_EQ("a123"_as.toUInt(), std::nullopt);
}

TEST(Strings, FindChar) {
    EXPECT_EQ(L"abcabc"_as.find('a'), 0);
    EXPECT_EQ(L"abcabc"_as.find('b'), 1);
    EXPECT_EQ(L"abcabc"_as.find('c'), 2);
    EXPECT_EQ(L"abcabc"_as.find('?'), -1);
}

TEST(Strings, RFindChar) {
    EXPECT_EQ(L"abcabc"_as.rfind('a'), 3);
    EXPECT_EQ(L"abcabc"_as.rfind('b'), 4);
    EXPECT_EQ(L"abcabc"_as.rfind('c'), 5);
    EXPECT_EQ(L"abcabc"_as.rfind('d'), -1);
}

TEST(Strings, FindString) {
    EXPECT_EQ(L"abcabc"_as.find(L"abc"), 0);
    EXPECT_EQ(L"abcabc"_as.find(L"abc", 1), 1);
}

TEST(Strings, RFindString) {
    EXPECT_EQ(L"abcabc"_as.rfind('a'), 3);
    EXPECT_EQ(L"abcabc"_as.rfind('b'), 4);
    EXPECT_EQ(L"abcabc"_as.rfind('c'), 5);
    EXPECT_EQ(L"abcabc"_as.rfind('d'), -1);
}

TEST(Strings, TrimLeft) {
    EXPECT_EQ(L" abc"_as.trimLeft(), "abc");
    EXPECT_EQ(L" abc "_as.trimLeft(), "abc ");
    EXPECT_EQ(L" абв "_as.trimLeft(), "абв ");
    EXPECT_EQ(L"abc"_as.trimLeft(), "abc");
}
