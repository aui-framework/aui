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

AWindow::~AWindow() {
    mRenderingContext->destroyNativeWindow(*this);
}


void AWindow::quit() {
    getWindowManager().mWindows.removeIf([this](const auto& p) {
        return p.get() == this;
    });

    IPlatformAbstraction::current().windowQuit(*this);

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
    if (!mHandle) return;
    IPlatformAbstraction::current().windowSetStyle(*this, ws);
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

void AWindow::show() {
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

void AWindow::setSize(glm::ivec2 size) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, size.x, size.y);
    IPlatformAbstraction::current().windowSetSize(*this, size);
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

void AWindow::hide() {
    IPlatformAbstraction::current().windowHide(*this);
}

void AWindowManager::notifyProcessMessages() {
    if (mWindows.empty()) {
        return;
    }
    IPlatformAbstraction::current().windowManagerNotifyProcessMessages();
}

void AWindowManager::loop() {
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
void AWindow::applyGeometryToChildren() {
    ASurface::applyGeometryToChildren();
}
