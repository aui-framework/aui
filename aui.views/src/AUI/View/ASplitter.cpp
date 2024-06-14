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
    AViewContainer::onPointerPressed(event);
    if (getViewAt(event.position) == nullptr) {
        mHelper.beginDrag(event.position);
    }
}


void ASplitter::setSize(glm::ivec2 size) {
    AViewContainer::setSize(size);
}

void ASplitter::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AViewContainer::onPointerMove(pos, event);
    if (mHelper.mouseDrag(pos)) {
        updateLayout();
        redraw();
    }
}

void ASplitter::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
    mHelper.endDrag();
}

void ASplitter::onClickPrevented() {
    AViewContainer::onClickPrevented();
    mHelper.endDrag();
}
