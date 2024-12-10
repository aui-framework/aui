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

AGridSplitter::AGridSplitter()
  : mHorizontalHelper(ALayoutDirection::HORIZONTAL), mVerticalHelper(ALayoutDirection::VERTICAL) {}

glm::bvec2 AGridSplitter::isDraggingArea(glm::ivec2 position) {
    return { mHorizontalHelper.isDraggingArea(position), mVerticalHelper.isDraggingArea(position) };
}

void AGridSplitter::onPointerPressed(const APointerPressedEvent& event) {
    auto isDrag = isDraggingArea(event.position);
    if (isDrag.y) {
        mVerticalHelper.beginDrag(event.position);
    }
    if (isDrag.x) {
        mHorizontalHelper.beginDrag(event.position);
    }
    if (glm::any(isDrag)) {
        AView::onPointerPressed(event); // NOLINT(*-parent-virtual-call)
        return;
    }
    AViewContainerBase::onPointerPressed(event);
}

void AGridSplitter::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    mVerticalHelper.mouseDrag(pos);
    mHorizontalHelper.mouseDrag(pos);

    if (mVerticalHelper.isDragging() || mHorizontalHelper.isDragging()) {
        applyGeometryToChildrenIfNecessary();
        redraw();
        AView::onPointerMove(pos, event); // NOLINT(*-parent-virtual-call)
        return;
    }
    if (!isPressed()) {
        auto area = isDraggingArea(pos);
        if (area.x && area.y) {
            setCursor(ACursor::MOVE);
            AView::onPointerMove(pos, event);   // NOLINT(*-parent-virtual-call)
            return;
        }
        if (area.x) {
            setCursor(ACursor::EW_RESIZE);
            AView::onPointerMove(pos, event);   // NOLINT(*-parent-virtual-call)
            return;
        }
        if (area.y) {
            setCursor(ACursor::NS_RESIZE);
            AView::onPointerMove(pos, event);   // NOLINT(*-parent-virtual-call)
            return;
        }
    }
    setCursor({});
    AViewContainerBase::onPointerMove(pos, {});
}

void AGridSplitter::onPointerReleased(const APointerReleasedEvent& event) {
    if (mVerticalHelper.isDragging() || mHorizontalHelper.isDragging()) {
        mVerticalHelper.endDrag();
        mHorizontalHelper.endDrag();
        AView::onPointerReleased(event); // NOLINT(*-parent-virtual-call)
        return;
    }

    AViewContainerBase::onPointerReleased(event);
}

void AGridSplitter::updateSplitterItems() {
    setLayout(std::make_unique<AAdvancedGridLayout>(mItems.first().size(), mItems.size()));
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

bool AGridSplitter::consumesClick(const glm::ivec2& pos) { return true; }
