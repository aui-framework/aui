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


void ::AStackedLayout::layout(int x, int y, int width, int height) {
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

int ::AStackedLayout::onComputeIntrinsicWidth(int height) {
    int m = 0;
    for (auto& v: mViews) {
      if (!!(v->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
        m = glm::max(v->computeWidth(height == -1 ? -1 : std::max(0, height - v->getMargin().vertical())) + v->getMargin().horizontal(), m);
      }
    }
    return m;
}

int ::AStackedLayout::onComputeIntrinsicHeight(int width) {
    int m = 0;
    for (auto& v: mViews) {
        if (!!(v->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
          m = glm::max(v->computeHeight(width == -1 ? -1 : std::max(0, width - v->getMargin().horizontal())) + v->getMargin().vertical(), m);
        }
    }
    return m;
}
