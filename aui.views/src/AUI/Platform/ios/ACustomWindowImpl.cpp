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

#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/ADesktop.h"
#include <cstring>
#include <AUI/View/AButton.h>

ACustomWindow::ACustomWindow(const AString& name, int width, int height) :
        AWindow(name, width, height) {


    setWindowStyle(WindowStyle::NO_DECORATORS);
}
void ACustomWindow::onPointerPressed(const APointerPressedEvent& event) {
    if (event.position.y < mTitleHeight) {
        if (isCaptionAt(event.position)) {
            // TODO apple

            mDragging = true;
            mDragPos = event.position;
            emit dragBegin(event.position);
        }
    }
    AViewContainer::onPointerPressed(event);
}

void ACustomWindow::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
}
void ACustomWindow::handleXConfigureNotify() {
    emit dragEnd();

    AViewContainer::onPointerReleased({mDragPos});
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
