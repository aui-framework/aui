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

#include "AAbsoluteLayout.h"

void AAbsoluteLayout::layout(int x, int y, int width, int height) {
    for (const auto& i : mViews) {
        if (i.pivotX && i.pivotY) {
            i.view->layout(
                i.pivotX->getValuePx() + x, i.pivotY->getValuePx() + y,
                i.sizeX ? i.sizeX->getValuePx() : i.view->computeMinMaxSizes().max.x,
                i.sizeY ? i.sizeY->getValuePx() : i.view->computeMinMaxSizes().max.y);
            continue;
        }
        i.view->layout(i.view->getPosition(), i.view->getSize());
    }
}

glm::ivec2 AAbsoluteLayout::onIntrinsicMeasure(AConstraints constraints) {
    glm::ivec2 size = {};
    for (const auto& i : mViews) {
        const int x = i.pivotX.valueOr(AMetric(0)).getValuePx();
        const int y = i.pivotY.valueOr(AMetric(0)).getValuePx();
        const auto measured = i.view->measure({
            .minWidth = 0,
            .minHeight = 0,
            .maxWidth = i.sizeX ? static_cast<int>(i.sizeX->getValuePx()) : constraints.maxWidth,
            .maxHeight = i.sizeY ? static_cast<int>(i.sizeY->getValuePx()) : constraints.maxHeight,
        });
        size.x = glm::max(size.x, x + (i.sizeX ? static_cast<int>(i.sizeX->getValuePx()) : measured.x));
        size.y = glm::max(size.y, y + (i.sizeY ? static_cast<int>(i.sizeY->getValuePx()) : measured.y));
    }
    return size;
}

AMinMaxSizes AAbsoluteLayout::onComputeIntrinsicMinMaxSizes(int) {
    AMinMaxSizes result;
    for (const auto& i : mViews) {
        const auto minMax = i.view->computeMinMaxSizes();
        const int x = i.pivotX.valueOr(AMetric(0)).getValuePx();
        const int y = i.pivotY.valueOr(AMetric(0)).getValuePx();
        result.min.x = glm::max(result.min.x, x + (i.sizeX ? static_cast<int>(i.sizeX->getValuePx()) : minMax.min.x));
        result.min.y = glm::max(result.min.y, y + (i.sizeY ? static_cast<int>(i.sizeY->getValuePx()) : minMax.min.y));
        result.max.x = glm::max(result.max.x, x + (i.sizeX ? static_cast<int>(i.sizeX->getValuePx()) : minMax.max.x));
        result.max.y = glm::max(result.max.y, y + (i.sizeY ? static_cast<int>(i.sizeY->getValuePx()) : minMax.max.y));
    }
    return result;
}

void AAbsoluteLayout::add(aui::detail::AbsoluteLayoutCell cell) {
    mViews << std::move(cell);
    requestLayout();
}

void AAbsoluteLayout::addView(const _<AView>& view, AOptional<size_t> index) {
    addViewBasicImpl({
        .view = view
    }, index);
}
