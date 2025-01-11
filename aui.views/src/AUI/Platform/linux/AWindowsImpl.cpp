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
    getWindowManager().mWindows.removeIf([this](const auto& p) {
        return p.get() == this;
    });

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
    if (!mHandle) return;
    if (!!(ws & (WindowStyle::SYS | WindowStyle::NO_DECORATORS))) {
    }
}

float AWindow::fetchDpiFromSystem() const {
    return APlatform::getDpiRatio();
}

void AWindow::restore() {

}

void AWindow::minimize() {
    if (!mHandle) return;
}

bool AWindow::isMinimized() const {
    if (!mHandle) return false;
    return false;
}


bool AWindow::isMaximized() const {
    if (!mHandle) return false;
    return false;
}

void AWindow::maximize() {
    if (!mHandle) return;
    // https://github.com/glfw/glfw/blob/master/src/x11_window.c#L2355
}

glm::ivec2 AWindow::getWindowPosition() const {
    if (!mHandle) return {0, 0};
    return {};
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
    if (mHandle) {
        AThread::current() << [&]() {
//            XMapWindow(CommonRenderingContext::ourDisplay, mHandle);
        };
    }

    emit shown();
}

void AWindow::setSize(glm::ivec2 size) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, size.x, size.y);
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize({width, height});

    if (!mHandle) return;

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
    if (!mHandle) return;
}

void AWindowManager::notifyProcessMessages() {
    if (mWindows.empty()) {
        return;
    }
    if (mFastPathNotify.exchange(true)) {
        return;
    }
    char dummy = 0;
    int unused = write(mNotifyPipe.in(), &dummy, sizeof(dummy));
}

void AWindowManager::loop() {
    // make sure notifyProcessMessages would not block as it syscalls write to the pipe
}

void AWindow::blockUserInput(bool blockUserInput) {
    AWindowBase::blockUserInput(blockUserInput);
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
void AWindow::applyGeometryToChildren() {
    AWindowBase::applyGeometryToChildren();
}
