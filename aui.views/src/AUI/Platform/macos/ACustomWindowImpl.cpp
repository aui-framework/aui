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

#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/ADesktop.h"
#include <cstring>
#include <AUI/View/AButton.h>


ACustomWindow::ACustomWindow(const AString& name, int width, int height, AWindow* parent) :
    AWindow(name, width, height, parent) {
    setWindowStyle(WindowStyle::NO_DECORATORS);
}

void ACustomWindow::onPointerPressed(const APointerPressedEvent& event) {
    AViewContainer::onPointerPressed(event);
}

void ACustomWindow::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
}
void ACustomWindow::handleXConfigureNotify() {
    emit dragEnd();

    // x11 does not send release button event
    AViewContainer::onPointerReleased({mDragPos, APointerIndex::button(AInput::LBUTTON)});
}

bool ACustomWindow::isCaptionAt(const glm::ivec2& pos) {
    if (pos.y <= mTitleHeight) {
        if (auto v = getViewAtRecursive(pos)) {
            if (!(_cast<AButton>(v)) &&
                !v->getAssNames().contains(".override-title-dragging")) {
                return true;
            }
        }
    }
    return false;
}

void ACustomWindow::setSize(glm::ivec2 size) { AWindow::setSize(size); }
