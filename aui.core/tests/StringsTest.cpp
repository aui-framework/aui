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

    EXPECT_NEAR("123"_as.toFloat().valueOr(0), 123, ABS_ERROR);
    EXPECT_NEAR("123.456"_as.toFloat().valueOr(0), 123.456, ABS_ERROR);
    EXPECT_NEAR("-123.456"_as.toFloat().valueOr(0), -123.456, ABS_ERROR);
    EXPECT_NEAR("-0.0"_as.toFloat().valueOr(99999), 0, ABS_ERROR);
    EXPECT_EQ("123abs"_as.toUInt(), std::nullopt);
    EXPECT_EQ("1a23"_as.toUInt(), std::nullopt);
    EXPECT_EQ("a123"_as.toUInt(), std::nullopt);
}

TEST(Strings, ReplaceAll1) {
    EXPECT_EQ("abcdef"_as.replaceAll("ab", "12"), "12cdef");
}

TEST(Strings, ReplaceAll2) {
    EXPECT_EQ("abcdef"_as.replaceAll("ab", "1234"), "1234cdef");
}

TEST(Strings, ReplaceAll3) {
    EXPECT_EQ("abcdef"_as.replaceAll("abcd", "1"), "1ef");
}

TEST(Strings, ReplaceAll4) {
    EXPECT_EQ("abcdef"_as.replaceAll("abcd", ""), "ef");
}

TEST(Strings, ReplaceAll5) {
    EXPECT_EQ("abcdef"_as.replaceAll("b", "12"), "a12cdef");
}

TEST(Strings, ReplaceAll6) {
    EXPECT_EQ("abcdef"_as.replaceAll("b", "1234"), "a1234cdef");
}

TEST(Strings, ReplaceAll7) {
    EXPECT_EQ("abcdef"_as.replaceAll("bcd", "1"), "a1ef");
}

TEST(Strings, ReplaceAll8) {
    EXPECT_EQ("abcdef"_as.replaceAll("bcd", ""), "aef");
}
