// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "AUI/GL/gl.h"
#import <Cocoa/Cocoa.h>
#include "AUI/GL/GLDebug.h"
#include "AUI/Common/AString.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/Render/Render.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Util/ARandom.h"
#include "AUI/GL/State.h"
#include "AUI/Thread/AThread.h"
#include "AUI/GL/OpenGLRenderer.h"
#include "AUI/Platform/Platform.h"
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
    getWindowManager().mWindows.removeFirst(_cast<AWindow>(sharedPtr()));

    AThread::current()->enqueue([&]() {
        mSelfHolder = nullptr;
    });
    if (getWindowManager().mWindows.empty()) {
        MacosApp::inst().quit();
    }
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
    setGeometry(getWindowPosition().x, getWindowPosition().y, size.x, size.y);
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize(size);

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

void AWindow::requestTouchscreenKeyboard() {
    ABaseWindow::requestTouchscreenKeyboard();
}


void AWindow::hideTouchscreenKeyboard() {
    ABaseWindow::hideTouchscreenKeyboard();
}
