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

#include "AUI/GL/gl.h"
#include "AUI/Platform/AWindow.h"
#import <Cocoa/Cocoa.h>
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Render/IRenderer.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/OpenGLRenderingContext.h"
#import "MacosApp.h"

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>

#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>



AWindow::~AWindow() {
    mRenderingContext->destroyNativeWindow(*this);
}


void AWindow::quit() {
    if (!mHandle) {
        return;
    }
    getWindowManager().mWindows.removeFirst(mSelfHolder);

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
    [static_cast<NSWindow*>(mHandle) close];
    mHandle = nullptr;
    if (getWindowManager().mWindows.empty()) {
        MacosApp::inst().quit();
    }
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
    AThread::current() << [&]() {
        redraw();
    };

    auto ns = static_cast<NSWindow*>(mHandle);
    [ns orderFront:nil];
    [ns setIsVisible:YES];
    MacosApp::inst().activateIgnoringOtherApps();

    emit shown();
}
void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
    if (!mHandle) return;
    if (!!(ws & (WindowStyle::SYS | WindowStyle::NO_DECORATORS))) {
    /*
        breaks text input and looks poor
        auto s = static_cast<NSWindow*>(mHandle);
        [s setStyleMask:NSWindowStyleMaskBorderless];
        [s setTitlebarAppearsTransparent:YES];
        [s setTitleVisibility:NSWindowTitleHidden];*/
    }
}

float AWindow::fetchDpiFromSystem() const {
    return float([static_cast<NSWindow*>(mHandle) backingScaleFactor]);
}

void AWindow::restore() {

}

void AWindow::minimize() {
}

bool AWindow::isMinimized() const {
    return false;
}


bool AWindow::isMaximized() const {
    return false;
}

void AWindow::maximize() {

}

glm::ivec2 AWindow::getWindowPosition() const {
    return {0, 0};
}


void AWindow::flagRedraw() {
    if (auto crc = dynamic_cast<CommonRenderingContext*>(mRenderingContext.get())) {
        mRedrawFlag = true;
        crc->requestFrame();
    }
}


void AWindow::setSize(glm::ivec2 size) {
    auto s = static_cast<NSWindow*>(mHandle);
    float dpi = getDpiRatio();
    [s setMinSize:NSSizeFromCGSize({getMinimumWidth() / dpi, getMinimumHeight() / dpi})];
    [s setMaxSize:NSSizeFromCGSize({getMaxSize().x / dpi, getMaxSize().y / dpi})];
    [s setContentSize:NSSizeFromCGSize({size.x / dpi, size.y / dpi})];
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize({width, height});
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
    return position - getWindowPosition();
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
    return position + getWindowPosition();
}


void AWindow::setIcon(const AImage& image) {
}

void AWindow::hide() {
}

void AWindow::blockUserInput(bool blockUserInput) {

}

void AWindowManager::notifyProcessMessages() {
    dispatch_async(dispatch_get_main_queue(), ^{
        AThread::processMessages();
    });
}

void AWindowManager::loop() {
    MacosApp::inst().run();
}

void AWindow::allowDragNDrop() {

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
