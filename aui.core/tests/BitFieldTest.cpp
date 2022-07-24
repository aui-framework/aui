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

#include <AUI/Util/ABitField.h>
#include <AUI/Util/EnumUtil.h>


AUI_ENUM_FLAG(Flags) {
    FLAG1 = 1 << 0,
    FLAG2 = 1 << 1,
    FLAG3 = 1 << 2,
    FLAG23 = FLAG2 | FLAG3
};

TEST(Bitfield, FlagsCombination) {
    Flags f = Flags::FLAG1 | Flags::FLAG2;
    EXPECT_TRUE(f & Flags::FLAG1);
    EXPECT_TRUE(f & Flags::FLAG2);
    EXPECT_FALSE(f & Flags::FLAG3);
}

TEST(Bitfield, FlagCombinationInDeclaration) { // checks Flags::FLAG23 definition
    EXPECT_FALSE(Flags::FLAG23 & Flags::FLAG1);
    EXPECT_TRUE(Flags::FLAG23 & Flags::FLAG2);
    EXPECT_TRUE(Flags::FLAG23 & Flags::FLAG3);

}


TEST(Bitfield, Put) {
    ABitField<Flags> f;
    f << Flags::FLAG1;
    Flags x = f;
    ASSERT_TRUE((x & Flags::FLAG1));
    ASSERT_TRUE((!(x & ~Flags::FLAG1)));
    f << Flags::FLAG2;
    x = f;
    ASSERT_TRUE((x & Flags::FLAG1));
    ASSERT_TRUE((x & Flags::FLAG2));
    ASSERT_TRUE(((x & ~Flags::FLAG1)));
    ASSERT_TRUE(((x & ~Flags::FLAG2)));
}

TEST(Bitfield, Take) {
    ABitField<Flags> f;
    f << Flags::FLAG1 << Flags::FLAG2;
    Flags x = f;
    ASSERT_TRUE((x & Flags::FLAG1));
    ASSERT_TRUE((x & Flags::FLAG2));
    f >> Flags::FLAG2;
    x = f;
    ASSERT_TRUE((x & Flags::FLAG1));
    ASSERT_TRUE((!(x & Flags::FLAG2)));
}

TEST(Bitfield, CheckTake1) {
    ABitField<Flags> f;
    f << Flags::FLAG1 << Flags::FLAG2;
    Flags x = f;
    ASSERT_TRUE((x & Flags::FLAG1));
    ASSERT_TRUE((x & Flags::FLAG2));
    ASSERT_TRUE(f.checkAndSet(Flags::FLAG2));
    x = f;
    ASSERT_TRUE((x & Flags::FLAG1));
    ASSERT_TRUE((!(x & Flags::FLAG2)));
}
TEST(Bitfield, CheckTake2) {
    ABitField<Flags> f;
    f << Flags::FLAG1;
    Flags x = f;
    ASSERT_TRUE((x & Flags::FLAG1));
    ASSERT_TRUE((!(x & Flags::FLAG2)));
    ASSERT_TRUE(!f.checkAndSet(Flags::FLAG2));
    x = f;
    ASSERT_TRUE((x & Flags::FLAG1));
    ASSERT_TRUE((!(x & Flags::FLAG2)));
}
TEST(Bitfield, Check) {
    ABitField<Flags> f;
    f << Flags::FLAG1;

    ASSERT_TRUE(((f.check(Flags::FLAG1) && (f & Flags::FLAG1))));
    ASSERT_TRUE(!f.check(Flags::FLAG2));
    ASSERT_TRUE(!f.check(Flags::FLAG3));

    f << Flags::FLAG2;

    ASSERT_TRUE(f.check(Flags::FLAG1));
    ASSERT_TRUE(f.check(Flags::FLAG2));
    ASSERT_TRUE(!f.check(Flags::FLAG3));

    f >> Flags::FLAG3;
    f >> Flags::FLAG1;

    ASSERT_TRUE(!f.check(Flags::FLAG1));
    ASSERT_TRUE(f.check(Flags::FLAG2));
    ASSERT_TRUE(!f.check(Flags::FLAG3));
}

