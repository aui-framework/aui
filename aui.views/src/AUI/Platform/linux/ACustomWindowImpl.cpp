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
#include <AUI/Util/AMetric.h>

namespace {
using ResizeEdge = IPlatformAbstraction::ResizeEdge;

/**
 * @return edge of a window which does not have resize borders by the window manager, if pos is close to it.
 */
AOptional<ResizeEdge> resizeEdgeAt(AWindow& window, glm::ivec2 pos) {
    if (!!(window.windowStyle() & WindowStyle::NO_RESIZE) || window.isMaximized()) {
        return std::nullopt;
    }
    auto& pa = IPlatformAbstraction::current();
    if (pa.windowHasNativeResizeBorders(window)) {
        return std::nullopt;
    }
    const int border = int((6_dp).getValuePx());
    const int corner = int((12_dp).getValuePx());
    const auto size = window.getSize();
    const bool left = pos.x < border, right = pos.x >= size.x - border;
    const bool top = pos.y < border, bottom = pos.y >= size.y - border;
    const bool nearLeft = pos.x < corner, nearRight = pos.x >= size.x - corner;
    const bool nearTop = pos.y < corner, nearBottom = pos.y >= size.y - corner;
    if ((top && nearLeft) || (left && nearTop)) return ResizeEdge::TOP_LEFT;
    if ((top && nearRight) || (right && nearTop)) return ResizeEdge::TOP_RIGHT;
    if ((bottom && nearLeft) || (left && nearBottom)) return ResizeEdge::BOTTOM_LEFT;
    if ((bottom && nearRight) || (right && nearBottom)) return ResizeEdge::BOTTOM_RIGHT;
    if (top) return ResizeEdge::TOP;
    if (bottom) return ResizeEdge::BOTTOM;
    if (left) return ResizeEdge::LEFT;
    if (right) return ResizeEdge::RIGHT;
    return std::nullopt;
}

ACursor::System cursorOf(ResizeEdge edge) {
    switch (edge) {
        case ResizeEdge::TOP_LEFT:
        case ResizeEdge::BOTTOM_RIGHT:
            return ACursor::NWSE_RESIZE;
        case ResizeEdge::TOP_RIGHT:
        case ResizeEdge::BOTTOM_LEFT:
            return ACursor::NESW_RESIZE;
        case ResizeEdge::TOP:
        case ResizeEdge::BOTTOM:
            return ACursor::NS_RESIZE;
        case ResizeEdge::LEFT:
        case ResizeEdge::RIGHT:
            return ACursor::EW_RESIZE;
    }
    return ACursor::DEFAULT;
}
}   // namespace

ACustomWindow::ACustomWindow(const AString& name, AMetric width, AMetric height, AWindow* parent) :
        // NO_TITLEBAR must be known at native window creation: gtk installs client-side decorations at that point.
        AWindow(name, width, height, parent, WindowStyle::NO_TITLEBAR) {
    setWindowStyle(WindowStyle::NO_TITLEBAR);
}

void ACustomWindow::onPointerPressed(const APointerPressedEvent& event) {
    if (event.asButton == AInput::LBUTTON) {
        if (auto edge = resizeEdgeAt(*this, event.position)) {
            IPlatformAbstraction::current().windowBeginResize(*this, *edge);
            return;
        }
    }
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

void ACustomWindow::onPointerMove(glm::vec2 pos, const APointerMoveEvent& event) {
    AWindow::onPointerMove(pos, event);
    if (auto edge = resizeEdgeAt(*this, pos)) {
        ACursor(cursorOf(*edge)).applyNativeCursor(this);
    }
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
