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

#include <AUI/Util/ABitField.h>
#include "AUI/Reflect/AEnumerate.h"


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

