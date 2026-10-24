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
#include "AUI/Platform/detail/WindowGeometry.h"
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
#include <AUI/View/ATitlebar.h>
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
    destroyNativeWindow();
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
    AThread::current() << [&]() {
        redraw();
    };

    // no native window (i.e., AStubWindowManager in tests): don't touch AppKit, it would turn the process into a
    // foreground application (dock icon, menu bar) and steal the focus
    if (auto ns = static_cast<NSWindow*>(mHandle)) {
        [ns orderFront:nil];
        [ns setIsVisible:YES];
        MacosApp::inst().activateIgnoringOtherApps();
    }

    emit shown();
}
void AWindow::setTitleImpl(const AString& title) {
    if (!mHandle) return;
    [static_cast<NSWindow*>(mHandle) setTitle:[NSString stringWithUTF8String:title.toStdString().c_str()]];
    // AppKit relayouts the title text field.
    applyTitlebarRect();
}

void AWindow::setResizable(bool resizable) {
    mResizableCache = resizable;
    if (!mHandle || mClosed) return;
    auto s = static_cast<NSWindow*>(mHandle);
    NSWindowStyleMask mask = [s styleMask];
    if (resizable) {
        mask |= NSWindowStyleMaskResizable;
    } else {
        mask &= ~NSWindowStyleMaskResizable;
    }
    [s setStyleMask:mask];
}

bool AWindow::isResizable() const {
    if (!mHandle) return mResizableCache && !mClosed;
    return ([static_cast<NSWindow*>(mHandle) styleMask] & NSWindowStyleMaskResizable) != 0;
}

bool AWindow::hasNativeWindowControls() const { return true; }

bool AWindow::hasNativeWindowTitle() const {
    return !mIsPopup && std::holds_alternative<Init::DecorationExtended>(mInit.decoration);
}

bool AWindow::captionPressed(const APointerPressedEvent& event) {
    if (!mHandle || event.asButton != AInput::LBUTTON || !isCaptionAt(glm::ivec2(event.position))) {
        return false;
    }
    auto* ns = static_cast<NSWindow*>(mHandle);
    NSEvent* current = [NSApp currentEvent];
    if (!current) {
        return false;
    }
    if ([current clickCount] >= 2) {
        if (isResizable()) {
            [ns performZoom:nil];
        }
        return true;
    }
    [ns performWindowDragWithEvent:current];
    return true;
}

void AWindow::applyTitlebarRect() {
    // Aligns the traffic lights and the title vertically with the ATitlebar; AppKit resets their position on resize,
    // title change and fullscreen transitions, and we layout on each of them.
    if (!mHandle || !hasNativeWindowTitle()) {
        return;
    }
    auto* ns = static_cast<NSWindow*>(mHandle);

    // the native title is shown only while there is AWindowTitle to show it over.
    auto* title = mTitlebar ? mTitlebar->windowTitle() : nullptr;
    const bool titleVisible = title && title->getVisibility() == Visibility::VISIBLE;
    const auto visibility = titleVisible ? NSWindowTitleVisible : NSWindowTitleHidden;
    if ([ns titleVisibility] != visibility) {
        [ns setTitleVisibility:visibility];
    }
    if (!mTitlebar) {
        return;
    }

    const float dpi = getDpiRatio();
    const auto position = mTitlebar->getPositionInWindow();
    const auto size = mTitlebar->getSize();
    const CGFloat contentHeight = [[ns contentView] bounds].size.height;
    // center of the titlebar in the window coordinates (points, origin at the bottom left)
    const CGFloat centerY = contentHeight - (position.y + size.y / 2.f) / dpi;
    const auto alignVertically = [&](NSView* view) {
        NSView* container = [view superview];
        const NSPoint inContainer = [container convertPoint:NSMakePoint(0, centerY) fromView:nil];
        [view setFrameOrigin:NSMakePoint([view frame].origin.x, inContainer.y - [view frame].size.height / 2)];
    };
    NSView* titlebarView = nil;
    for (auto type : { NSWindowCloseButton, NSWindowMiniaturizeButton, NSWindowZoomButton }) {
        NSButton* button = [ns standardWindowButton:type];
        if (!button) continue;
        titlebarView = [button superview];
        if ([button isHidden]) continue;
        alignVertically(button);
    }
    if (titleVisible && titlebarView) {
        // the title text field is a sibling of the traffic lights. AppKit keeps centering it horizontally.
        for (NSView* subview in [titlebarView subviews]) {
            if ([subview isKindOfClass:[NSTextField class]]) {
                alignVertically(subview);
            }
        }
    }
    updateWindowControlsInsets();
}

void AWindow::updateWindowControlsInsets() {
    if (!mHandle || !std::holds_alternative<Init::DecorationExtended>(mInit.decoration)) {
        return;
    }
    auto* ns = static_cast<NSWindow*>(mHandle);
    const float dpi = getDpiRatio();
    CGFloat right = 0;
    for (auto type : { NSWindowCloseButton, NSWindowMiniaturizeButton, NSWindowZoomButton }) {
        NSButton* button = [ns standardWindowButton:type];
        if (!button || [button isHidden]) continue;
        const NSRect inWindow = [button convertRect:[button bounds] toView:nil];
        right = std::max(right, NSMaxX(inWindow));
    }
    const CGFloat titlebarHeight = [ns frame].size.height - [ns contentLayoutRect].size.height;
    windowControlsInsets = AWindowControlsInsets {
        .left = right > 0 ? int(std::ceil((right + 8) * dpi)) : 0,
        .right = 0,
        .height = int(std::ceil(titlebarHeight * dpi)),
    };
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
    if (!mHandle) return {0, 0};
    auto* s = static_cast<NSWindow*>(mHandle);
    const NSRect frame = [s frame];
    NSScreen* primary = [[NSScreen screens] firstObject];
    if (!primary) return {0, 0};
    const CGFloat primaryH = [primary frame].size.height;
    const float dpi = float([s backingScaleFactor]);
    // AppKit stores frames in points with bottom-left origin; AUI wants physical-pixel
    // top-left origin relative to the primary display.
    const int x = int(frame.origin.x * dpi);
    const int y = int((primaryH - (frame.origin.y + frame.size.height)) * dpi);
    return {x, y};
}


void AWindow::flagRedraw() {
    if (auto crc = dynamic_cast<CommonRenderingContext*>(mRenderingContext.get())) {
        mRedrawFlag = true;
        crc->requestFrame();
    }
}

void AWindow::setGeometry(int x, int y, int width, int height) {
    AViewContainer::setPosition({x, y});
    AViewContainer::setSize({width, height});
    if (!mHandle) return;
    auto* s = static_cast<NSWindow*>(mHandle);
    NSScreen* primary = [[NSScreen screens] firstObject];
    if (!primary) return;
    const CGFloat primaryH = [primary frame].size.height;
    // real pixels <-> points, see WindowDelegate windowDidResize:
    const float dpi = [s backingScaleFactor];
    const auto minSize = getMinimumSize();
    [s setContentMinSize:NSSizeFromCGSize({aui::platform::detail::pixelsToPoints(minSize.x, dpi),
                                           aui::platform::detail::pixelsToPoints(minSize.y, dpi)})];
    // -1 means "unlimited" (see AView::getMaxSize); dividing it by dpi would pass a negative size to AppKit, which
    // then clamps the window to 0x0.
    const auto maxSize = getMaxSize();
    [s setContentMaxSize:NSSizeFromCGSize({aui::platform::detail::maxPixelsToPoints(maxSize.x, dpi),
                                           aui::platform::detail::maxPixelsToPoints(maxSize.y, dpi)})];

    // width/height describe the content area; x/y is the top-left corner of the whole window frame.
    const double contentW = aui::platform::detail::pixelsToPoints(width, dpi);
    const double contentH = aui::platform::detail::pixelsToPoints(height, dpi);
    const NSSize frameSize = [s frameRectForContentRect:NSMakeRect(0, 0, contentW, contentH)].size;
    // Flip AUI top-left origin back to AppKit bottom-left for setFrame.
    const NSRect frame = NSMakeRect(x / dpi, primaryH - y / dpi - frameSize.height, frameSize.width, frameSize.height);
    if (!NSEqualRects(frame, [s frame])) {
        [s setFrame:frame display:YES];
    }
}

glm::ivec2 AWindow::mapPosition(const glm::ivec2& position) {
    return position - getWindowPosition();
}
glm::ivec2 AWindow::unmapPosition(const glm::ivec2& position) {
    return position + getWindowPosition();
}


void AWindow::setIcon(const AImage& image) {
}

void AWindow::hideImpl() {
    if (!mHandle) return;
    [static_cast<NSWindow*>(mHandle) orderOut:nil];
}

void AWindow::activate(const AString&) {
    show();
    if (mClosed) return;
    if (auto ns = static_cast<NSWindow*>(mHandle)) {
        if ([ns isMiniaturized]) {
            [ns deminiaturize:nil];
        }
        [ns makeKeyAndOrderFront:nil];
        MacosApp::inst().activateIgnoringOtherApps();
    }
}

void AWindow::blockUserInput(bool blockUserInput) {
    // the modal child window is attached to this window (see CommonRenderingContext::init); AUIView ignores input of
    // blocked windows.
    mInputBlocked = blockUserInput;
}

void AWindowManager::notifyProcessMessages() {
    auto self = this;
    dispatch_async(dispatch_get_main_queue(), ^{
        AThread::processMessages();
        // holds released / quit() requested
        if (self->mLoopRunning && !self->shouldKeepRunning()) {
            MacosApp::inst().quit();
        }
    });
}

void AWindowManager::loop() {
    mLoopRunning = true;
    if (!shouldKeepRunning()) {
        return;
    }
    MacosApp::inst().run();
    mLoopRunning = false;
}

void AWindow::allowDragNDrop() {

}

void AWindow::showTouchscreenKeyboardImpl() {
    ASurface::showTouchscreenKeyboardImpl();
}


void AWindow::hideTouchscreenKeyboardImpl() {
    ASurface::hideTouchscreenKeyboardImpl();
}

void AWindow::moveToCenter() {
    if (!mHandle) return;
    auto* s = static_cast<NSWindow*>(mHandle);
    NSScreen* screen = [s screen];
    if (!screen) {
        screen = [[NSScreen screens] firstObject];
    }
    if (!screen) return;

    const NSRect screenFrame = [screen frame];
    const float dpi = getDpiRatio();
    const glm::ivec2 windowSize = getSize();

    const CGFloat centerX = screenFrame.origin.x + (screenFrame.size.width - windowSize.x / dpi) / 2;
    const CGFloat centerY = screenFrame.origin.y + (screenFrame.size.height - windowSize.y / dpi) / 2;

    NSScreen* primary = [[NSScreen screens] firstObject];
    const CGFloat primaryH = [primary frame].size.height;
    const int auiX = int(centerX * dpi);
    const int auiY = int((primaryH - (centerY + windowSize.y / dpi)) * dpi);

    setPosition({auiX, auiY});
}

void AWindow::setMobileScreenOrientation(AScreenOrientation screenOrientation) {

}
