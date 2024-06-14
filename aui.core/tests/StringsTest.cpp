//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

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

TEST(Strings, Uppercase1) {
    EXPECT_EQ("å"_as.uppercase(), "Å");
}

TEST(Strings, Uppercase2) {
    EXPECT_EQ("àáâäǎæãāăą ŵëþűųǐíïıįğ çżð"_as.uppercase(), "ÀÁÂÄǍÆÃĀĂĄ ŴËÞŰŲǏÍÏİĮĞ ÇŻÐ");
}

TEST(Strings, Downcase1) {
    EXPECT_EQ("Å"_as.lowercase(), "å");
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
