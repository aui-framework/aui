/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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


void AWindow::setSize(int width, int height) {
    setGeometry(getWindowPosition().x, getWindowPosition().y, width, height);
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize(width, height);

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
    dispatch_async(dispatch_get_main_queue(), ^{
        AThread::processMessages();
    });
}

void AWindowManager::loop() {
    MacosApp::inst().run();
}
