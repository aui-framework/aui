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

#import <Cocoa/Cocoa.h>


ACustomWindow::ACustomWindow(const AString& name, int width, int height, AWindow* parent) :
    AWindow(name, width, height, parent) {
    setWindowStyle(WindowStyle::NO_TITLEBAR);
}

namespace {
// A click is a caption drag candidate when its deepest view sits inside a `.window-title`
// ancestor and is not itself an interactive widget. This ignores mTitleHeight entirely,
// which is brittle at HiDPI (literal 30px regardless of scale) and mismatches the actual
// caption row height set by the impl.
bool shouldDragCaption(ACustomWindow* self, const glm::ivec2& pos) {
    auto v = self->getViewAtRecursive(pos);
    if (!v) return false;
    if (_cast<AButton>(v)) return false;
    if (v->getAssNames().contains(".override-title-dragging")) return false;
    for (AView* cur = v.get(); cur; cur = cur->getParent()) {
        if (cur->getAssNames().contains(".window-title")) {
            return true;
        }
    }
    return false;
}
}

void ACustomWindow::onPointerPressed(const APointerPressedEvent& event) {
    if (mHandle && event.asButton == AInput::LBUTTON && shouldDragCaption(this, event.position)) {
        auto* ns = static_cast<NSWindow*>(mHandle);
        if (NSEvent* current = [NSApp currentEvent]) {
            const bool canZoom = !(mWindowStyle & WindowStyle::NO_RESIZE);
            if ([current clickCount] >= 2) {
                if (canZoom) {
                    [ns performZoom:nil];
                }
                return;
            }
            [ns performWindowDragWithEvent:current];
            return;
        }
    }
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
