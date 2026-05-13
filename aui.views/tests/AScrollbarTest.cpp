/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by Nelonn on 5/13/2026.
//

#include <gtest/gtest.h>
#include <AUI/View/AScrollbar.h>

TEST(AScrollbar, MeasureIsStableForZeroMainAxisConstraint) {
  AScrollbar horizontal(ALayoutDirection::HORIZONTAL);
  AScrollbar vertical(ALayoutDirection::VERTICAL);

  const auto horizontalMeasured = horizontal.measure(AConstraints::fixedInline(0));
  const auto verticalMeasured = vertical.measure(AConstraints::fixedBlock(0));

  EXPECT_EQ(horizontalMeasured.x, 0);
  EXPECT_GT(horizontalMeasured.y, 0);
  EXPECT_EQ(verticalMeasured.y, 0);
  EXPECT_GT(verticalMeasured.x, 0);
}
