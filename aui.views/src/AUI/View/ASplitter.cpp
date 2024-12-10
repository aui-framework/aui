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

#include "ASplitter.h"
#include <AUI/Util/UIBuildingHelpers.h>

ASplitter::ASplitter() {
}

void ASplitter::onPointerPressed(const APointerPressedEvent& event) {
    if (mHelper.isDraggingArea(event.position)) {
        AView::onPointerPressed(event); // NOLINT(*-parent-virtual-call)
        mHelper.beginDrag(event.position);
        return;
    }
    AViewContainerBase::onPointerPressed(event);
}


void ASplitter::setSize(glm::ivec2 size) {
    AViewContainerBase::setSize(size);
}

void ASplitter::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    if (mHelper.isDragging() || (mHelper.isDraggingArea(pos) && !isPressed())) {
        setCursor(mHelper.mDirection == ALayoutDirection::HORIZONTAL ? ACursor::EW_RESIZE : ACursor::NS_RESIZE);
        AView::onPointerMove(pos, event); // NOLINT(*-parent-virtual-call)
        if (mHelper.mouseDrag(pos)) {
            applyGeometryToChildrenIfNecessary();
            redraw();
        }
        return;
    }
    setCursor({});
    AViewContainerBase::onPointerMove(pos, event);
}

void ASplitter::onPointerReleased(const APointerReleasedEvent& event) {
    if (mHelper.isDragging()) {
        AView::onPointerReleased(event); // NOLINT(*-parent-virtual-call)
        mHelper.endDrag();
        return;
    }
    AViewContainerBase::onPointerReleased(event);
}

void ASplitter::onClickPrevented() {
    AViewContainerBase::onClickPrevented();
    mHelper.endDrag();
}
