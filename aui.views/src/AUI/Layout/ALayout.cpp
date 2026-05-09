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

AMinMaxSizes ALayout::computeMinMaxSizes(int height) {
    if (auto it = mMinMaxSizesCache.find(height); it != mMinMaxSizesCache.end()) {
        return it->second;
    }
    return mMinMaxSizesCache.emplace(height, onComputeIntrinsicMinMaxSizes(height)).first->second;
}

glm::ivec2 ALayout::measure(AConstraints constraints) {
    if (auto it = mMeasureCache.find(constraints); it != mMeasureCache.end()) {
        return it->second;
    }

    AConstraints effectiveConstraints = constraints;
    const bool unlimitedWidth = effectiveConstraints.maxWidth == -1;
    const bool unlimitedHeight = effectiveConstraints.maxHeight == -1;
    const int effectiveMaxWidth = unlimitedWidth
        ? std::numeric_limits<int>::max()
        : std::max(effectiveConstraints.minWidth, effectiveConstraints.maxWidth);
    const int effectiveMaxHeight = unlimitedHeight
        ? std::numeric_limits<int>::max()
        : std::max(effectiveConstraints.minHeight, effectiveConstraints.maxHeight);

    effectiveConstraints.maxWidth = unlimitedWidth ? -1 : effectiveMaxWidth;
    effectiveConstraints.maxHeight = unlimitedHeight ? -1 : effectiveMaxHeight;

    auto measuredSize = onIntrinsicMeasure(effectiveConstraints);
    measuredSize.x = std::clamp(measuredSize.x, effectiveConstraints.minWidth, effectiveMaxWidth);
    measuredSize.y = std::clamp(measuredSize.y, effectiveConstraints.minHeight, effectiveMaxHeight);

    return mMeasureCache.emplace(constraints, measuredSize).first->second;
}

int ALayout::getMinimumWidth() {
    return computeMinMaxSizes(-1).min.x;
}

int ALayout::getMinimumHeight() {
    return computeMinMaxSizes(-1).min.y;
}

void ALayout::setSpacing(int spacing) {}

ALayoutDirection ALayout::getLayoutDirection() { return ALayoutDirection::NONE; }
