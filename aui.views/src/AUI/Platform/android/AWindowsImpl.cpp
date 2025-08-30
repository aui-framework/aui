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
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/IRenderer.h"
#include "AUI/Platform/android/OSAndroid.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Platform/APlatform.h"
#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/OpenGLRenderingContext.h"

#include <chrono>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Traits/memory.h>
#include <AUI/Traits/strings.h>

#include <AUI/Action/AMenu.h>
#include <AUI/Util/AViewProfiler.h>
#include <AUI/View/AAbstractTextField.h>


AWindow::~AWindow() {
    mRenderingContext->destroyNativeWindow(*this);
}


void AWindow::quit() {
    getWindowManager().mWindows.removeFirst(_cast<AWindow>(aui::ptr::shared_from_this(this)));

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
}

void AWindow::setWindowStyle(WindowStyle ws) {
    mWindowStyle = ws;
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
    mRedrawFlag = true;
}

float AWindow::fetchDpiFromSystem() const {
    return APlatform::getDpiRatio();
}


void AWindow::setSize(glm::ivec2 size) {
    AUI_NULLSAFE(mRenderingContext)->beginResize(*this);
    AViewContainer::setSize(size);
    AUI_NULLSAFE(mRenderingContext)->endResize(*this);
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

void AWindowManager::notifyProcessMessages() {
    com::github::aui::android::Platform::requestRedraw();
}

void AWindowManager::loop() {
    AThread::processMessages();
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

    emit shown();
}

void AWindow::blockUserInput(bool block) {

}

void AWindow::allowDragNDrop() {

}

void AWindow::showTouchscreenKeyboardImpl() {
    AUI_UI_THREAD {
        ATextInputType type = ATextInputType::DEFAULT;
        ATextInputActionIcon action = ATextInputActionIcon::DEFAULT;
        bool isPassword = false;
        if (auto textField = _cast<AAbstractTypeable>(AWindow::getFocusedView())) {
            type = textField->textInputType();
            action = textField->textInputActionIcon();
            isPassword = textField->isPasswordField();
        }
        com::github::aui::android::Platform::showKeyboard(static_cast<int>(type),
                                                        static_cast<int>(action), isPassword);
    };
}

void AWindow::hideTouchscreenKeyboardImpl() {
    com::github::aui::android::Platform::hideKeyboard();
}

void AWindow::moveToCenter() {

}

void AWindow::setMobileScreenOrientation(AScreenOrientation screenOrientation) {
    com::github::aui::android::Platform::setMobileScreenOrientation(static_cast<int>(screenOrientation));
}
