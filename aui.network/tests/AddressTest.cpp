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

#include <gtest/gtest.h>
#include <AUI/Network/AInet4Address.h>

TEST(Inet4Address, FromArrayToString) {
    EXPECT_EQ(AInet4Address::fromArray({ 192, 168, 1, 2 }, 80).toString(), "192.168.1.2:80");
}

TEST(Inet4Address, FromArrayMatchesFromString) {
    EXPECT_EQ(AInet4Address::fromArray({ 127, 0, 0, 1 }, 8080), AInet4Address::fromString("127.0.0.1", 8080));
}
