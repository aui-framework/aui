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

TEST(AScrollbar, VerticalChildrenStayWithinCrossAxisBounds) {
  AScrollbar vertical(ALayoutDirection::VERTICAL);
  vertical.setScrollDimensions(50, 100);
  vertical.layout({ 0, 0 }, { 13, 100 });

  for (const auto& child : vertical.getViews()) {
    if (!(child->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
      continue;
    }
    EXPECT_GE(child->getPosition().x, 0);
    EXPECT_LE(child->getPosition().x + child->getWidth(), vertical.getWidth());
  }
}

TEST(AScrollbar, HorizontalChildrenStayWithinCrossAxisBounds) {
  AScrollbar horizontal(ALayoutDirection::HORIZONTAL);
  horizontal.setScrollDimensions(50, 100);
  horizontal.layout({ 0, 0 }, { 100, 13 });

  for (const auto& child : horizontal.getViews()) {
    if (!(child->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
      continue;
    }
    EXPECT_GE(child->getPosition().y, 0);
    EXPECT_LE(child->getPosition().y + child->getHeight(), horizontal.getHeight());
  }
}
