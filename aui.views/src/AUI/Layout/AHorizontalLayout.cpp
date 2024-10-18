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

#include "AHorizontalLayout.h"
#include "AUI/Common/AVector.h"
#include "AUI/View/AViewContainer.h"

#undef max
#undef min


void AHorizontalLayout::onResize(int x, int y, int width, int height) {
    if (mViews.empty()) return;

    int sum = 0;
    int availableSpace = width + mSpacing;

    for (auto& view: mViews) {
        view->ensureAssUpdated();
        if (view->getVisibility() == Visibility::GONE) continue;
        int expanding = view->getExpandingHorizontal();
        int minSpace = view->getMinimumWidth(ALayoutDirection::HORIZONTAL);
        sum += expanding;
        if (expanding == 0 || view->getFixedSize().x != 0)
            availableSpace -= minSpace + view->getMargin().horizontal() + mSpacing;
        else
            availableSpace -= view->getMargin().horizontal() + mSpacing;
    }

    bool containsExpandingItems = sum > 0;

    sum = glm::max(sum, 1);

    unsigned index = 0;

    int posX = x;
    auto last = mViews.back();
    for (auto& view: mViews) {
        if (view->getVisibility() == Visibility::GONE) continue;
        auto margins = view->getMargin();
        auto maxSize = view->getMaxSize();

        if (containsExpandingItems && view == last) {
            // the last element should stick right to the border.
            int viewPosX = posX + margins.left;
            int viewWidth = width - viewPosX - margins.right + x;
            view->setGeometry(viewPosX,
                              y + margins.top,
                              viewWidth,
                              height - margins.vertical());
        } else {
            int expanding = view->getExpandingHorizontal();
            int minSpace = view->getMinimumWidth(ALayoutDirection::HORIZONTAL);
            int viewWidth = glm::clamp(availableSpace * expanding / sum, minSpace, maxSize.x);

            view->setGeometry(posX + margins.left,
                              y + margins.top,
                              viewWidth,
                              glm::min(height - margins.vertical(), maxSize.y));
            posX += view->getSize().x + mSpacing + margins.horizontal();
            availableSpace += viewWidth - view->getSize().x;
        }
    }
    //assert(width == x);
}


int AHorizontalLayout::getMinimumWidth() {
    int minWidth = -mSpacing;

    for (auto& v: mViews) {
        if (v->getVisibility() == Visibility::GONE) continue;
        minWidth += v->getMinimumWidth(ALayoutDirection::HORIZONTAL) + mSpacing + v->getMargin().horizontal();
    }

    return glm::max(minWidth, 0);
}

int AHorizontalLayout::getMinimumHeight() {
    int minHeight = 0;
    for (auto& v: mViews) {
        if (v->getVisibility() == Visibility::GONE) continue;
        auto h = v->getMinimumHeight(ALayoutDirection::HORIZONTAL);
        minHeight = glm::max(minHeight, int(h + v->getMargin().vertical()));
    }
    return minHeight;
}

void AHorizontalLayout::setSpacing(int spacing) {
    if (mSpacing == spacing) [[unlikely]] {
        return;
    }
    mSpacing = spacing;
    if (mViews.empty()) {
        return;
    }
    AUI_NULLSAFE(mViews.first()->getParent())->markMinContentSizeInvalid();
}

ALayoutDirection AHorizontalLayout::getLayoutDirection() {
    return ALayoutDirection::HORIZONTAL;
}

