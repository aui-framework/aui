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
                i.sizeX ? i.sizeX->getValuePx() : i.view->computeWidth(-1),
                i.sizeY ? i.sizeY->getValuePx() : i.view->computeHeight(-1));
            continue;
        }
        i.view->layout(i.view->getPosition(), i.view->getSize());
    }
}

int AAbsoluteLayout::onComputeIntrinsicWidth(int height) {
    int v = 0;
    for (const auto& i : mViews) {
        int x = i.pivotX.valueOr(AMetric(0)).getValuePx();
        if (i.sizeX) {
            x += i.sizeX->getValuePx();
        } else {
            x += i.view->computeWidth(-1);
        }
        v = glm::max(v, x);
    }
    return v;
}

int AAbsoluteLayout::onComputeIntrinsicHeight(int width) {
    int v = 0;
    for (const auto& i : mViews) {
        int x = i.pivotY.valueOr(AMetric(0)).getValuePx();
        if (i.sizeY) {
            x += i.sizeY->getValuePx();
        } else {
            x += i.view->computeHeight(-1);
        }
        v = glm::max(v, x);
    }
    return v;
}

void AAbsoluteLayout::add(aui::detail::AbsoluteLayoutCell cell) {
    mViews << std::move(cell);
}

void AAbsoluteLayout::addView(const _<AView>& view, AOptional<size_t> index) {
    addViewBasicImpl({
        .view = view
    }, index);
}
