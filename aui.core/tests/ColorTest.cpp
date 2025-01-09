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

#include <gtest/gtest.h>
#include "AUI/Common/AColor.h"
#include "AUI/Common/AString.h"

TEST(Color, Hex) {
    EXPECT_EQ(AColor("#fff"), AColor::WHITE);
    EXPECT_EQ(AColor("#000"), AColor::BLACK);
}
