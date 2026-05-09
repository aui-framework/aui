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

static constexpr int UNBOUNDED_CONSTRAINT = 1000000;

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
    if (effectiveConstraints.maxWidth == -1) {
        effectiveConstraints.maxWidth = UNBOUNDED_CONSTRAINT;
    }
    if (effectiveConstraints.maxHeight == -1) {
        effectiveConstraints.maxHeight = UNBOUNDED_CONSTRAINT;
    }
    effectiveConstraints.maxWidth = std::max(effectiveConstraints.minWidth, effectiveConstraints.maxWidth);
    effectiveConstraints.maxHeight = std::max(effectiveConstraints.minHeight, effectiveConstraints.maxHeight);

    auto measuredSize = onIntrinsicMeasure(effectiveConstraints);
    measuredSize.x = std::clamp(measuredSize.x, effectiveConstraints.minWidth, effectiveConstraints.maxWidth);
    measuredSize.y = std::clamp(measuredSize.y, effectiveConstraints.minHeight, effectiveConstraints.maxHeight);

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
