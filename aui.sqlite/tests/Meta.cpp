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
#include <AUI/Data/ASqlDatabase.h>
#include <AUI/Data/AMeta.h>


TEST(Meta, Meta) {
    Autumn::put(ASqlDatabase::connect("sqlite", ":memory:"));
    AMeta::set("kek", 4);
    AMeta::set("lol", "azaza");
    ASSERT_EQ(AMeta::get("kek"), 4);
    ASSERT_EQ(AMeta::get("lol"), "azaza");
    AMeta::set("kek", "four");
    AMeta::set("lol", 42.0);
    ASSERT_EQ(AMeta::get("kek"), "four");
    ASSERT_EQ(AMeta::get("lol"), 42.0);
}

