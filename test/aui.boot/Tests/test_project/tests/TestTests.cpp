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
// Created by alex2 on 31.08.2020.
//

#include <gtest/gtest.h>
#include <AUI/Traits/algorithms.h>
#include <AUI/Common/AVector.h>

int someFunctionFromExecutable(int a, int b);

TEST(Test, Test1) {
    AVector<int> items = { 1, 2, 3 };
    ASSERT_TRUE(items.contains(2));
}
TEST(Test, Test2) {
    ASSERT_EQ(someFunctionFromExecutable(2, 2), 4);
}
