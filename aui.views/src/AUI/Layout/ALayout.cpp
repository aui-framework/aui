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

#include "ALinearLayout.h"

#include <limits>

void ALayout::requestLayout() {
  mMeasureCache.clear();
  mMinMaxSizesCache.clear();
}

AMinMaxAxis ALayout::computeMinMaxAxis(int height) {
  if (auto cached = mMinMaxSizesCache.get(height)) {
    return *cached;
  }
  return mMinMaxSizesCache.put(height, onComputeIntrinsicMinMaxAxis(height));
}

glm::ivec2 ALayout::measure(AConstraints constraints) {
  if (auto cached = mMeasureCache.get(constraints)) {
    return *cached;
  }

  AConstraints effectiveConstraints = constraints;
  const bool unlimitedInline = effectiveConstraints.maxInline == -1;
  const bool unlimitedBlock = effectiveConstraints.maxBlock == -1;
  const int effectiveMaxInline =
      unlimitedInline
          ? std::numeric_limits<int>::max()
          : std::max(effectiveConstraints.minInline, effectiveConstraints.maxInline);
  const int effectiveMaxBlock =
      unlimitedBlock
          ? std::numeric_limits<int>::max()
          : std::max(effectiveConstraints.minBlock, effectiveConstraints.maxBlock);

  effectiveConstraints.maxInline = unlimitedInline ? -1 : effectiveMaxInline;
  effectiveConstraints.maxBlock = unlimitedBlock ? -1 : effectiveMaxBlock;

  auto measuredSize = onIntrinsicMeasure(effectiveConstraints);
  measuredSize.x = std::clamp(measuredSize.x, effectiveConstraints.minInline, effectiveMaxInline);
  measuredSize.y = std::clamp(measuredSize.y, effectiveConstraints.minBlock, effectiveMaxBlock);

  return mMeasureCache.put(constraints, measuredSize);
}

int ALayout::getMinimumWidth() { return computeMinMaxAxis(-1).min; }

int ALayout::getMinimumHeight() { return measure(AConstraints::fixedInline(getMinimumWidth())).y; }

void ALayout::setSpacing(int spacing) {}

ALayoutDirection ALayout::getLayoutDirection() { return ALayoutDirection::NONE; }
