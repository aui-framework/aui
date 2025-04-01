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

//
// Created by Alex2772 on 12/9/2021.
//

#include <AUI/Platform/CommonRenderingContext.h>
#include "MacosApp.h"
#import "WindowDelegate.h"
#import "MainView.h"

void CommonRenderingContext::requestFrame() {
    if (!CVDisplayLinkIsRunning(static_cast<CVDisplayLinkRef>(mDisplayLink)))
        CVDisplayLinkStart(static_cast<CVDisplayLinkRef>(mDisplayLink));
}

void CommonRenderingContext::init(const Init& init) {
    auto& window = init.window;
    MacosApp::inst();
    auto delegate = [[WindowDelegate alloc] initWithWindow: &window];
    AUI_ASSERT(delegate != nullptr);
    auto windowRect = NSMakeRect(100, 100, init.width, init.height);
    NSWindowStyleMask windowStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
    NSWindow* nsWindow = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
    AUI_ASSERT(nsWindow != nullptr);
    window.mHandle = nsWindow;
    window.updateDpi();
    [nsWindow setContentSize:NSMakeSize(init.width / window.getDpiRatio(), init.height / window.getDpiRatio())];

    @try {
        auto view = [[MainView alloc] initWithWindow:&window];
        [nsWindow setContentView:view];
        [nsWindow makeFirstResponder:view];
        [nsWindow setDelegate:delegate];
        [nsWindow setAcceptsMouseMovedEvents:YES];
        [nsWindow setRestorable:NO];
        [nsWindow setTitle: [NSString stringWithUTF8String:(init.name.toStdString().c_str())]];
        [view release];
    } @catch (NSException* e) {
        throw AException([[e reason] UTF8String]);
    }
    
    mWindow = &window;


    CVDisplayLinkCreateWithActiveCGDisplays(reinterpret_cast<CVDisplayLinkRef*>(&mDisplayLink));
    CVDisplayLinkSetOutputCallback(reinterpret_cast<CVDisplayLinkRef>(mDisplayLink), [](CVDisplayLinkRef displayLink, const CVTimeStamp* _now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* ctx) -> CVReturn {
        auto myCtx = reinterpret_cast<CommonRenderingContext *>(ctx);
        if (!myCtx->mFrameScheduled)
        {
            std::shared_ptr<AView>* windowSharedPtr = nullptr;
            try {
                if (auto sharedPtr = myCtx->mWindow->sharedPtr()) {
                    windowSharedPtr = new std::shared_ptr(std::move(sharedPtr));
                } else {
                    return kCVReturnSuccess;
                }
            } catch(...) {
                return kCVReturnSuccess;
            }
            myCtx->mFrameScheduled = true;
            dispatch_async(dispatch_get_main_queue(), ^{
                AUI_DEFER { delete windowSharedPtr; };
                myCtx->mWindow->mRedrawFlag = false;
                myCtx->mWindow->redraw();
                myCtx->mFrameScheduled = false;
                if (!myCtx->mWindow->mRedrawFlag) {
                    // next redraw is not needed
                    CVDisplayLinkStop(static_cast<CVDisplayLinkRef>(myCtx->mDisplayLink));
                }
            });
        }
        return kCVReturnSuccess;
    }, this);


}

void CommonRenderingContext::destroyNativeWindow(AWindowBase& window) {
    AUI_ASSERT(mDisplayLink != nullptr);
    CVDisplayLinkStop(static_cast<CVDisplayLinkRef>(mDisplayLink));
    CVDisplayLinkRelease(static_cast<CVDisplayLinkRef>(mDisplayLink));
    mDisplayLink = nullptr;
    [static_cast<NSWindow*>(mWindow->nativeHandle()) close];
}

void CommonRenderingContext::beginPaint(AWindowBase& window) {
}

void CommonRenderingContext::endPaint(AWindowBase& window) {
}
