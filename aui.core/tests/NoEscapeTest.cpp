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
// Created by alex2772 on 2/25/22.
//

#include <gtest/gtest.h>
#include <AUI/Traits/values.h>

class FailsOnCopy {
private:
    int mValue;

public:
    FailsOnCopy(int value): mValue(value) {}
    FailsOnCopy(FailsOnCopy&&) = default;
    FailsOnCopy(const FailsOnCopy&) {
        EXPECT_TRUE(false) << "the value shouldn't have copied";
    }

    int value() {
        return mValue;
    }
    int valueConst() const {
        return mValue;
    }
};

void check(aui::no_escape<FailsOnCopy> obj, int value) {
    EXPECT_EQ(obj->value(), value);
}

void checkConst(aui::no_escape<const FailsOnCopy> obj, int value) {
    EXPECT_EQ(obj->valueConst(), value);
}

TEST(NoEscape, LValue) {
    FailsOnCopy failsOnCopy(228);
    check(failsOnCopy, 228);
}
TEST(NoEscape, ConstLValue) {
    const FailsOnCopy failsOnCopy(229);
    checkConst(failsOnCopy, 229);
}
TEST(NoEscape, RValue1) {
    check(FailsOnCopy(230), 230);
    checkConst(FailsOnCopy(231), 231);
}
TEST(NoEscape, RValue2) {
    auto func = [] {
        return FailsOnCopy(232);
    };

    check(func(), 232);
}