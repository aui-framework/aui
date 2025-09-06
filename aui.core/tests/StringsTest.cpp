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

TEST(Strings, Clown) {
    EXPECT_EQ("🤡"_as, "🤡");
    EXPECT_EQ("🤡"_as.toStdString(), "🤡");
}

TEST(Strings, MultibyteErase) {
    auto s = "A🤡B"_as;
    s.erase(1, 1);
    EXPECT_EQ(s, "AB");
    EXPECT_EQ(s.bytes().size(), 2);
}

TEST(Strings, MultibyteInsert) {
    auto s = "AB"_as;
    s.insert(1, U'🤡');
    EXPECT_EQ(s, "A🤡B");
    EXPECT_EQ(s.bytes().size(), 6);
}

TEST(Strings, Chinese) {
    EXPECT_EQ("嗨"_as, "嗨");
    EXPECT_EQ("嗨"_as.length(), 1);
    EXPECT_EQ("嗨"_as.toStdString(), "嗨");
}

TEST(Strings, Utf8Iterators) {
    AString str("Привет, 🤡, Как твои дела?");

    EXPECT_EQ(*(str.begin() + 8), U'🤡');
    EXPECT_EQ((str.begin() + 9) - (str.begin() + 7), 2);

    (str.begin() + 8) = U'👽';
    EXPECT_EQ(*(str.begin() + 8), U'👽');
    EXPECT_EQ((str.begin() + 9) - (str.begin() + 7), 2);

    (str.begin() + 8) = 'A';
    EXPECT_EQ(*(str.begin() + 8), 'A');
    EXPECT_EQ((str.begin() + 9) - (str.begin() + 7), 2);
}
