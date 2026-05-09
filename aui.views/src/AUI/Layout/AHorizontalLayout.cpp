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

#include "AHorizontalLayout.h"
#include "AUI/Common/AVector.h"
#include "AUI/View/AViewContainer.h"

#undef max
#undef min

#include "HVLayout.h"

using HVLayout = aui::HVLayout<ALayoutDirection::HORIZONTAL>;

void AHorizontalLayout::layout(int x, int y, int width, int height) {
    HVLayout::layout({ x, y }, { width, height }, mViews, mSpacing);
}

glm::ivec2 AHorizontalLayout::onIntrinsicMeasure(AConstraints constraints) {
    return HVLayout::onIntrinsicMeasure(mViews, mSpacing, constraints);
}

AMinMaxSizes AHorizontalLayout::onComputeIntrinsicMinMaxSizes(int) {
    return HVLayout::computeIntrinsicMinMaxSizes(mViews, mSpacing);
}

void AHorizontalLayout::setSpacing(int spacing) {
    if (mSpacing == spacing) [[unlikely]] {
        return;
    }
    mSpacing = spacing;
    requestLayout();
    if (mViews.empty()) {
        return;
    }
    AUI_NULLSAFE(mViews.first()->getParent())->requestLayout();
}

ALayoutDirection AHorizontalLayout::getLayoutDirection() {
    return ALayoutDirection::HORIZONTAL;
}
