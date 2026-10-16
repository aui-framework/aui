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
#include "AUI/Platform/CommonRenderingContext.h"
#include "AUI/Platform/linux/IPlatformAbstraction.h"
#include <AUI/View/AButton.h>

ACustomWindow::ACustomWindow(const AString& name, AMetric width, AMetric height, AWindow* parent) :
        AWindow(name, width, height, parent) {
    setWindowStyle(WindowStyle::NO_TITLEBAR);
}

void ACustomWindow::onPointerPressed(const APointerPressedEvent& event) {
    if (event.position.y < mTitleHeight && event.asButton == AInput::LBUTTON) {
        if (isCaptionAt(event.position)) {
            IPlatformAbstraction::current().windowBeginMove(*this);

            mDragging = true;
            mDragPos = event.position;
            emit dragBegin(event.position);
        }
    }
    AWindow::onPointerPressed(event);
}


void ACustomWindow::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
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
