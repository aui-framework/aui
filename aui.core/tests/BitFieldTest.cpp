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

#include <AUI/Util/ABitField.h>
#include <AUI/Util/EnumUtil.h>


AUI_ENUM_FLAG(Flags) {
    FLAG1 = 0x1,
    FLAG2 = 0x2,
    FLAG3 = 0x4,
};

TEST(Bitfield, Put) {
    ABitField<Flags> f;
    f << Flags::FLAG1;
    Flags x = f;
    ASSERT_TRUE(!!(x & Flags::FLAG1));
    ASSERT_TRUE(!!(!(x & ~Flags::FLAG1)));
    f << Flags::FLAG2;
    x = f;
    ASSERT_TRUE(!!(x & Flags::FLAG1));
    ASSERT_TRUE(!!(x & Flags::FLAG2));
    ASSERT_TRUE(!!((x & ~Flags::FLAG1)));
    ASSERT_TRUE(!!((x & ~Flags::FLAG2)));
}

TEST(Bitfield, Take) {
    ABitField<Flags> f;
    f << Flags::FLAG1 << Flags::FLAG2;
    Flags x = f;
    ASSERT_TRUE(!!(x & Flags::FLAG1));
    ASSERT_TRUE(!!(x & Flags::FLAG2));
    f >> Flags::FLAG2;
    x = f;
    ASSERT_TRUE(!!(x & Flags::FLAG1));
    ASSERT_TRUE(!!(!(x & Flags::FLAG2)));
}

TEST(Bitfield, CheckTake1) {
    ABitField<Flags> f;
    f << Flags::FLAG1 << Flags::FLAG2;
    Flags x = f;
    ASSERT_TRUE(!!(x & Flags::FLAG1));
    ASSERT_TRUE(!!(x & Flags::FLAG2));
    ASSERT_TRUE(f.checkAndSet(Flags::FLAG2));
    x = f;
    ASSERT_TRUE(!!(x & Flags::FLAG1));
    ASSERT_TRUE(!!(!(x & Flags::FLAG2)));
}
TEST(Bitfield, CheckTake2) {
    ABitField<Flags> f;
    f << Flags::FLAG1;
    Flags x = f;
    ASSERT_TRUE(!!(x & Flags::FLAG1));
    ASSERT_TRUE(!!(!(x & Flags::FLAG2)));
    ASSERT_TRUE(!f.checkAndSet(Flags::FLAG2));
    x = f;
    ASSERT_TRUE(!!(x & Flags::FLAG1));
    ASSERT_TRUE(!!(!(x & Flags::FLAG2)));
}
TEST(Bitfield, Check) {
    ABitField<Flags> f;
    f << Flags::FLAG1;

    ASSERT_TRUE(!!((f.test(Flags::FLAG1) && (f & Flags::FLAG1))));
    ASSERT_TRUE(!f.test(Flags::FLAG2));
    ASSERT_TRUE(!f.test(Flags::FLAG3));

    f << Flags::FLAG2;

    ASSERT_TRUE(f.test(Flags::FLAG1));
    ASSERT_TRUE(f.test(Flags::FLAG2));
    ASSERT_TRUE(!f.test(Flags::FLAG3));

    f >> Flags::FLAG3;
    f >> Flags::FLAG1;

    ASSERT_TRUE(!f.test(Flags::FLAG1));
    ASSERT_TRUE(f.test(Flags::FLAG2));
    ASSERT_TRUE(!f.test(Flags::FLAG3));
}

