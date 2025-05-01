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

#include <X11/Xlib.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#include "AUI/GL/gl.h"
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Platform/CommonRenderingContext.h"
#include "AUI/Platform/ErrorToException.h"
#include "AUI/Render/IRenderer.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/OpenGLRenderingContext.h"
#include "AUI/UITestState.h"
#include "IPlatformAbstraction.h"

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>

#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>

#include <X11/extensions/sync.h>



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
    return APlatform::getDpiRatio();
}

void AWindow::restore() {
    IPlatformAbstraction::current().windowRestore(*this);
}

void AWindow::minimize() {
    IPlatformAbstraction::current().windowMinimize(*this);
}

bool AWindow::isMinimized() const {
    return IPlatformAbstraction::current().windowIsMinimized(*this);
}


bool AWindow::isMaximized() const {
    return IPlatformAbstraction::current().windowIsMaximized(*this);
}

void AWindow::maximize() {
    IPlatformAbstraction::current().windowMaximize(*this);
}

glm::ivec2 AWindow::getWindowPosition() const {
    return IPlatformAbstraction::current().windowGetPosition(*this);
}

void AWindow::flagRedraw() {
    mRedrawFlag = true;
}

void AWindow::show() {
    if (!getWindowManager().mWindows.contains(_cast<AWindow>(sharedPtr()))) {
        getWindowManager().mWindows << _cast<AWindow>(sharedPtr());
    }
    try {
        mSelfHolder = _cast<AWindow>(sharedPtr());
    } catch (...) {
        mSelfHolder = nullptr;
    }
    IPlatformAbstraction::current().windowShow(*this);

    emit shown();
}

void AWindow::setSize(glm::ivec2 size) {
    IPlatformAbstraction::current().windowSetSize(*this, size);
}

void AWindow::setGeometry(int x, int y, int width, int height) {
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
    IPlatformAbstraction::current().notifyProcessMessages();
}

void AWindowManager::loop() {
    IPlatformAbstraction::current().windowManagerLoop();
}

void AWindow::blockUserInput(bool blockUserInput) {
    IPlatformAbstraction::current().windowBlockUserInput(*this, blockUserInput);
}

void AWindow::allowDragNDrop() {
    IPlatformAbstraction::current().allowDragNDrop(*this);
}

void AWindow::showTouchscreenKeyboardImpl() {
    AWindowBase::showTouchscreenKeyboardImpl();
}

void AWindow::hideTouchscreenKeyboardImpl() {
    AWindowBase::hideTouchscreenKeyboardImpl();
}

void AWindow::moveToCenter() {

}

void AWindow::setMobileScreenOrientation(AScreenOrientation screenOrientation) {

}
void AWindow::applyGeometryToChildren() {
    AWindowBase::applyGeometryToChildren();
}
