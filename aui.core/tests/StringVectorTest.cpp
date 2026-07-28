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
#include "AUI/Common/AColor.h"
#include "AUI/Common/AString.h"
#include "AUI/Common/AStringVector.h"

TEST(StringVector, Split1) {
    EXPECT_EQ("1x2"_as.split('x'), AStringVector({"1", "2"}));
}

TEST(StringVector, Split2) {
    EXPECT_EQ("1"_as.split('x'), AStringVector({"1"}));
}

TEST(StringVector, Split3) {
    EXPECT_EQ(""_as.split('x'), AStringVector({}));
}
