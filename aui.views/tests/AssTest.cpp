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

#include <gmock/gmock.h>
#include <AUI/ASS/ASS.h>

using namespace ass;

TEST(AssTest, Merge1) {
    BackgroundImage rule1{ "icon.svg" };
    BackgroundImage rule2{ {}, AColor::RED };

    rule1 = rule2;
    EXPECT_EQ(std::get<AString>(*rule1.image), "icon.svg");
    EXPECT_EQ(*rule1.overlayColor, AColor::RED);
}
