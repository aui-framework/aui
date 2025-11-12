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


#include <emscripten/html5.h>

#include "AUI/GL/gl.h"
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/IRenderer.h"

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
}



namespace aui::emscripten {
    void applySize(ASurface* window, glm::dvec2 size) {
        auto s = glm::ivec2(size * emscripten_get_device_pixel_ratio());
        ALogger::info("Emscripten") << "px size: " << s;
        emscripten_set_canvas_element_size("#canvas", s.x, s.y);
        AUI_NULLSAFE(window)->setGeometry(0, 0, s.x, s.y);
    }
}

namespace {

    auto position(const EmscriptenMouseEvent* mouseEvent) {
        return glm::dvec2{mouseEvent->targetX, mouseEvent->targetY} * emscripten_get_device_pixel_ratio();
    }

    bool onResize(int eventType, const EmscriptenUiEvent* event, void *userData) {
        aui::emscripten::applySize(AWindow::current(), glm::dvec2{event->windowInnerWidth, event->windowInnerHeight});
        return true;
    }

    bool onMouseMove(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
        AUI_NULLSAFE(AWindow::current())->onPointerMove(position(mouseEvent), { .pointerIndex = APointerIndex::button((AInput::Key)mouseEvent->button), });
        return true;
    }


    bool onMousePressed(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
        AUI_NULLSAFE(AWindow::current())->onPointerPressed({
            .position = position(mouseEvent),
            .pointerIndex = APointerIndex::button((AInput::Key)mouseEvent->button),
            .asButton = AInput::LBUTTON,
        });
        return true;
    }


    bool onMouseReleased(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
        AUI_NULLSAFE(AWindow::current())->onPointerReleased({
            .position = position(mouseEvent),
            .pointerIndex = APointerIndex::button((AInput::Key)mouseEvent->button),
            .asButton = AInput::LBUTTON,
        });
        return true;
    }
}

void AWindowManager::loop() {
    AUI_DO_ONCE {
        emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false, onResize);
        emscripten_set_mousemove_callback("#canvas", nullptr, false, onMouseMove);
        emscripten_set_mousedown_callback("#canvas", nullptr, false, onMousePressed);
        emscripten_set_mouseup_callback("#canvas", nullptr, false, onMouseReleased);
    }


    emscripten_set_main_loop_arg([](void* arg) {
        auto self = (AWindowManager*)arg;
        AThread::processMessages();
        if (AWindow::isRedrawWillBeEfficient()) {
            for (auto& window: self->mWindows) {
                if (window->mRedrawFlag) {
                    window->mRedrawFlag = false;
                    AWindow::currentWindowStorage() = window.get();
                    self->mWatchdog.runOperation([&] {
                        window->redraw();
                    });
                }
            }
        }
    }, this, 0, true);
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
}

void AWindow::hideTouchscreenKeyboardImpl() {
}

void AWindow::moveToCenter() {

}

void AWindow::setMobileScreenOrientation(AScreenOrientation screenOrientation) {
}
