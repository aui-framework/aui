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

#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/CommonRenderingContext.h"
#include "AUI/Thread/AThread.h"
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Platform/OpenGLRenderingContext.h"
#include "IPlatformAbstraction.h"
#include <AUI/Util/AViewProfiler.h>
#include <AUI/View/ATitlebar.h>

AWindow::~AWindow() {
    destroyNativeWindow();
}

void AWindow::setResizable(bool resizable) {
    if (mClosed) {
        return;
    }
    IPlatformAbstraction::current().windowSetResizable(*this, resizable);
}

bool AWindow::isResizable() const {
    if (mClosed) {
        return false;
    }
    return IPlatformAbstraction::current().windowIsResizable(const_cast<AWindow&>(*this));
}

void AWindow::setTitleImpl(const AString& title) {
    IPlatformAbstraction::current().windowSetTitle(*this, title);
}

float AWindow::fetchDpiFromSystem() const {
    return IPlatformAbstraction::current().windowGetDpiRatio(const_cast<AWindow&>(*this));
}

void AWindow::restore() {
    IPlatformAbstraction::current().windowRestore(*this);
}

void AWindow::minimize() {
    IPlatformAbstraction::current().windowMinimize(*this);
}

bool AWindow::isMinimized() const {
    return IPlatformAbstraction::current().windowIsMinimized(const_cast<AWindow&>(*this));
}


bool AWindow::isMaximized() const {
    return IPlatformAbstraction::current().windowIsMaximized(const_cast<AWindow&>(*this));
}

void AWindow::maximize() {
    IPlatformAbstraction::current().windowMaximize(*this);
}

glm::ivec2 AWindow::getWindowPosition() const {
    return IPlatformAbstraction::current().windowGetPosition(const_cast<AWindow&>(*this));
}

void AWindow::flagRedraw() {
    if (mRedrawFlag) {
        return;
    }
    mRedrawFlag = true;
    IPlatformAbstraction::current().windowFlagRedraw(*this);
}

void AWindow::showImpl() {
    if (!getWindowManager().mWindows.contains(_cast<AWindow>(aui::ptr::shared_from_this(this)))) {
        getWindowManager().mWindows << _cast<AWindow>(aui::ptr::shared_from_this(this));
    }
    try {
        mSelfHolder = _cast<AWindow>(aui::ptr::shared_from_this(this));
    } catch (...) {
        mSelfHolder = nullptr;
    }
    IPlatformAbstraction::current().windowShow(*this);

    emit shown();
}

void AWindow::activate(const AString& activationToken) {
    show();
    if (mClosed) {
        return;
    }
    IPlatformAbstraction::current().windowActivate(*this, activationToken);
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({ x, y });
    AViewContainer::setSize({ width, height });

    IPlatformAbstraction::current().windowSetGeometry(*this, x, y, width, height);
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
    return position - getWindowPosition();
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
    return position + getWindowPosition();
}


void AWindow::setIcon(const AImage& image) {
    IPlatformAbstraction::current().windowSetIcon(*this, image);
}

void AWindow::hideImpl() {
    IPlatformAbstraction::current().windowHide(*this);
}

void AWindowManager::notifyProcessMessages() {
    if (mWindows.empty() && !mLoopRunning) {
        // nothing to wake up; also avoids initializing platform abstraction by embedded (i.e., SDL) applications.
        return;
    }
    IPlatformAbstraction::current().windowManagerNotifyProcessMessages();
}

void AWindowManager::loop() {
    mLoopRunning = true;
    if (!shouldKeepRunning()) {
        // nothing holds the application (CLI or embedded app); don't even touch the platform.
        return;
    }
    IPlatformAbstraction::current().windowManagerLoop();
}

void AWindow::blockUserInput(bool blockUserInput) {
    IPlatformAbstraction::current().windowBlockUserInput(*this, blockUserInput);
}

void AWindow::allowDragNDrop() {
    IPlatformAbstraction::current().windowAllowDragNDrop(*this);
}

void AWindow::showTouchscreenKeyboardImpl() {
    ASurface::showTouchscreenKeyboardImpl();
}

void AWindow::hideTouchscreenKeyboardImpl() {
    ASurface::hideTouchscreenKeyboardImpl();
}

void AWindow::moveToCenter() {

}

void AWindow::setMobileScreenOrientation(AScreenOrientation screenOrientation) {

}

bool AWindow::hasNativeWindowControls() const {
    return IPlatformAbstraction::current().windowHasNativeWindowControls(const_cast<AWindow&>(*this));
}

namespace {
using ResizeEdge = IPlatformAbstraction::ResizeEdge;

/**
 * @return edge of a window which does not have resize borders by the window manager, if pos is close to it.
 */
AOptional<ResizeEdge> resizeEdgeAt(AWindow& window, glm::ivec2 pos) {
    if (window.isPopup() || !window.isResizable() || window.isMaximized()) {
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

void AWindow::applyResizeEdgeCursor(glm::vec2 pos) {
    if (auto edge = resizeEdgeAt(*this, glm::ivec2(pos))) {
        ACursor(cursorOf(*edge)).applyNativeCursor(this);
    }
}

bool AWindow::captionPressed(const APointerPressedEvent& event) {
    if (event.asButton != AInput::LBUTTON) {
        return false;
    }
    auto& pa = IPlatformAbstraction::current();
    if (auto edge = resizeEdgeAt(*this, glm::ivec2(event.position))) {
        mWindowManagerDragging = true;
        pa.windowBeginResize(*this, *edge);
        return true;
    }
    if (isCaptionAt(glm::ivec2(event.position))) {
        const auto now = std::chrono::steady_clock::now();
        if (now - mLastCaptionPress < std::chrono::milliseconds(400)) {
            mLastCaptionPress = {};
            if (isResizable()) {
                if (isMaximized()) {
                    restore();
                } else {
                    maximize();
                }
            }
            return true;
        }
        mLastCaptionPress = now;
        mWindowManagerDragging = true;
        pa.windowBeginMove(*this);
        // the pointer press still reaches the views (i.e., to focus a text field in the titlebar).
    }
    return false;
}
