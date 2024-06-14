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

//
// Created by Alex2772 on 11/11/2021.
//

#include <AUI/Layout/AAdvancedGridLayout.h>
#include <AUI/Util/ALayoutDirection.h>
#include "AGridSplitter.h"

static const auto CLICK_BIAS = 8_dp;

AGridSplitter::AGridSplitter():
    mHorizontalHelper(ALayoutDirection::HORIZONTAL),
    mVerticalHelper(ALayoutDirection::VERTICAL)
{

}

void AGridSplitter::onPointerPressed(const APointerPressedEvent& event) {
    AViewContainer::onPointerPressed(event);
    auto mousePos = event.position;
    bool doVerticalDrag = true;
    for (auto& r : mItems) {
        auto& v = r.first();
        auto viewPos = v->getPosition().y + CLICK_BIAS.getValuePx();
        auto viewSize = v->getSize().y - CLICK_BIAS.getValuePx() * 2.f;

        if (mousePos.y > viewPos && mousePos.y < viewPos + viewSize) {
            doVerticalDrag = false;
            break;
        }

        if (mousePos.y <= viewPos) {
            break;
        }
    }
    bool doHorizontalDrag = true;
    for (auto& v : mItems.first()) {
        auto viewPos = v->getPosition().x + CLICK_BIAS.getValuePx();
        auto viewSize = v->getSize().x - CLICK_BIAS.getValuePx() * 2.f;

        if (mousePos.x > viewPos && mousePos.x < viewPos + viewSize) {
            doHorizontalDrag = false;
            break;
        }

        if (mousePos.x <= viewPos) {
            break;
        }
    }
    if (doVerticalDrag) {
        mVerticalHelper.beginDrag(mousePos);
    }
    if (doHorizontalDrag) {
        mHorizontalHelper.beginDrag(mousePos);
    }
}

void AGridSplitter::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AViewContainer::onPointerMove(pos, {});
    mVerticalHelper.mouseDrag(pos);
    mHorizontalHelper.mouseDrag(pos);

    if (mVerticalHelper.isDragging() || mHorizontalHelper.isDragging()) {
        updateLayout();
        redraw();
    }
}

void AGridSplitter::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
    mVerticalHelper.endDrag();
    mHorizontalHelper.endDrag();
}

void AGridSplitter::updateSplitterItems() {
    setLayout(_new<AAdvancedGridLayout>(mItems.first().size(), mItems.size()));
    AVector<_<AView>> horizontal;
    AVector<_<AView>> vertical;

    for (auto& row : mItems) {
        for (auto& v : row) {
            addView(v);
        }
        vertical << row.first();
    }
    for (auto& v : mItems.first()) {
        horizontal << v;
    }

    mVerticalHelper.setItems(std::move(vertical));
    mHorizontalHelper.setItems(std::move(horizontal));
}

bool AGridSplitter::consumesClick(const glm::ivec2& pos) {
    return true;
}
