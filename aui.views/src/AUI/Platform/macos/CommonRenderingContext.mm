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
    assert(delegate != nullptr);
    auto windowRect = NSMakeRect(100, 100, init.width, init.height);
    NSWindowStyleMask windowStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
    NSWindow* nsWindow = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
    assert(nsWindow != nullptr);
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
            myCtx->mFrameScheduled = true;
            dispatch_async(dispatch_get_main_queue(), ^{
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

void CommonRenderingContext::destroyNativeWindow(ABaseWindow& window) {
    assert(mDisplayLink != nullptr);
    CVDisplayLinkStop(static_cast<CVDisplayLinkRef>(mDisplayLink));
    CVDisplayLinkRelease(static_cast<CVDisplayLinkRef>(mDisplayLink));
    mDisplayLink = nullptr;
    [static_cast<NSWindow*>(mWindow->nativeHandle()) close];
}

void CommonRenderingContext::beginPaint(ABaseWindow& window) {
}

void CommonRenderingContext::endPaint(ABaseWindow& window) {
}
