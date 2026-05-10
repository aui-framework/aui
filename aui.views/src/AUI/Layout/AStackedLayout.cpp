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

#include "AStackedLayout.h"

void AStackedLayout::layout(int x, int y, int width, int height) {
  for (auto& v: mViews) {
    v->ensureAssUpdated();
    auto margins = v->getMargin();

    AConstraints constraints;
    if (v->getExpandingHorizontal() == 0) {
      constraints.minWidth = 0;
      constraints.maxWidth = std::max(0, width - margins.horizontal());
    } else {
      constraints.minWidth = constraints.maxWidth = std::max(0, width - margins.horizontal());
    }

    if (v->getExpandingVertical() == 0) {
      constraints.minHeight = 0;
      constraints.maxHeight = std::max(0, height - margins.vertical());
    } else {
      constraints.minHeight = constraints.maxHeight = std::max(0, height - margins.vertical());
    }

    auto measuredSize = v->measure(constraints);

    int finalWidth = measuredSize.x + margins.horizontal();
    int finalHeight = measuredSize.y + margins.vertical();

    int finalX = (width - finalWidth) / 2;
    int finalY = (height - finalHeight) / 2;

    v->layout(finalX + x + margins.left,
                   finalY + y + margins.top,
                   measuredSize.x,
                   measuredSize.y);
  }
}

glm::ivec2 AStackedLayout::onIntrinsicMeasure(AConstraints constraints) {
  int width = 0;
  int height = 0;
  for (auto& v: mViews) {
    if (!(v->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
      continue;
    }
    auto margins = v->getMargin().occupiedSize();
    auto measured = v->measure({
      .minWidth = 0,
      .maxWidth = constraints.isUnlimitedWidth() ? -1 : std::max(0, constraints.maxWidth - margins.x),
      .minHeight = 0,
      .maxHeight = constraints.isUnlimitedHeight() ? -1 : std::max(0, constraints.maxHeight - margins.y),
    });
    width = glm::max(width, measured.x + margins.x);
    height = glm::max(height, measured.y + margins.y);
  }
  return { width, height };
}

AMinMaxAxis AStackedLayout::onComputeIntrinsicMinMaxAxis(int) {
  AMinMaxAxis result;
  for (const auto& view : mViews) {
    if (!(view->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
      continue;
    }
    const auto minMax = view->computeMinMaxAxis();
    result.min = glm::max(result.min, minMax.min + view->getMargin().horizontal());
    result.max = glm::max(result.max, minMax.max + view->getMargin().horizontal());
  }
  return result;
}
