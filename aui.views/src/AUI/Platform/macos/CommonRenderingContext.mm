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
#include <AUI/Traits/callables.h>
#include <variant>
#include "MacosApp.h"
#import "WindowDelegate.h"
#import "AUIView.h"

void CommonRenderingContext::requestFrame() {
    if (!CVDisplayLinkIsRunning(static_cast<CVDisplayLinkRef>(mDisplayLink)))
        CVDisplayLinkStart(static_cast<CVDisplayLinkRef>(mDisplayLink));
}

void CommonRenderingContext::init(const Init& init) {
    @autoreleasepool {
        auto& window = init.window;
        MacosApp::inst();
        auto windowRect = NSMakeRect(100, 100, init.width, init.height);
        const auto& windowInit = window.init();
        NSWindowStyleMask windowStyle = NSWindowStyleMaskBorderless;
        bool transparentTitlebar = false;
        bool hasShadow = true;
        AWindow::Init::Buttons buttons;
        bool showButtons = true;
        if (!init.popup) {
            std::visit(
                aui::lambda_overloaded {
                  [&](const AWindow::Init::DecorationDefault& d) {
                      windowStyle = NSWindowStyleMaskTitled;
                      if (d.buttons.close) windowStyle |= NSWindowStyleMaskClosable;
                      if (d.buttons.minimize) windowStyle |= NSWindowStyleMaskMiniaturizable;
                      buttons = d.buttons;
                  },
                  [&](const AWindow::Init::DecorationExtended& d) {
                      // content extends into the titlebar; the native traffic lights are kept.
                      windowStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskFullSizeContentView;
                      if (d.buttons.close) windowStyle |= NSWindowStyleMaskClosable;
                      if (d.buttons.minimize) windowStyle |= NSWindowStyleMaskMiniaturizable;
                      transparentTitlebar = true;
                      buttons = d.buttons;
                  },
                  [&](const AWindow::Init::DecorationBorderless& d) {
                      if (d.shadow) {
                          // titled window without the titlebar: the system keeps the shadow and rounded corners.
                          windowStyle = NSWindowStyleMaskTitled | NSWindowStyleMaskFullSizeContentView;
                          transparentTitlebar = true;
                          showButtons = false;
                      } else {
                          windowStyle = NSWindowStyleMaskBorderless;
                          hasShadow = false;
                      }
                  },
                },
                windowInit.decoration);
            if (windowInit.resizable) {
                windowStyle |= NSWindowStyleMaskResizable;
            }
        }
        NSWindow* nsWindow = [[NSWindow alloc] initWithContentRect:windowRect styleMask:windowStyle backing:NSBackingStoreBuffered defer:NO];
        AUI_ASSERT(nsWindow != nullptr);
        window.mHandle = nsWindow;
        window.updateDpi();
        [nsWindow setContentSize:NSMakeSize(init.width / window.getDpiRatio(), init.height / window.getDpiRatio())];

        @try {
            auto view = [[AUIView alloc] initWithWindow:&window];
            [nsWindow setContentView:view];
            [nsWindow makeFirstResponder:view];
            auto delegate = [[WindowDelegate alloc] initWithWindow: &window];
            AUI_ASSERT(delegate != nullptr);
            [nsWindow setDelegate:delegate];
            [nsWindow setAcceptsMouseMovedEvents:YES];
            [nsWindow setRestorable:NO];
            [nsWindow setTitle: [NSString stringWithUTF8String:(init.name.c_str())]];
            if (init.popup) {
                // popup (context menu, dropdown, tooltip)
                [nsWindow setLevel:NSPopUpMenuWindowLevel];
                [nsWindow setHidesOnDeactivate:YES];
                [nsWindow setOpaque:NO];
                [nsWindow setBackgroundColor:[NSColor clearColor]];
                [nsWindow setHasShadow:YES];
            } else {
                [nsWindow setHasShadow:hasShadow];
                if (transparentTitlebar) {
                    [nsWindow setTitlebarAppearsTransparent:YES];
                    [nsWindow setTitleVisibility:NSWindowTitleHidden];
                }
                [[nsWindow standardWindowButton:NSWindowCloseButton] setHidden:!(showButtons && buttons.close)];
                [[nsWindow standardWindowButton:NSWindowMiniaturizeButton] setHidden:!(showButtons && buttons.minimize)];
                [[nsWindow standardWindowButton:NSWindowZoomButton] setHidden:!(showButtons && buttons.maximize && windowInit.resizable)];
                if (auto parent = window.parentWindow(); parent && parent->nativeHandle()) {
                    // keeps the child above its parent and moves them together
                    [static_cast<NSWindow*>(parent->nativeHandle()) addChildWindow:nsWindow ordered:NSWindowAbove];
                }
            }
        } @catch (NSException* e) {
            throw AException([[e reason] UTF8String]);
        }

        mWindow = &window;
        window.updateWindowControlsInsets();


        CVDisplayLinkCreateWithActiveCGDisplays(reinterpret_cast<CVDisplayLinkRef*>(&mDisplayLink));
        CVDisplayLinkSetOutputCallback(reinterpret_cast<CVDisplayLinkRef>(mDisplayLink), [](CVDisplayLinkRef displayLink, const CVTimeStamp* _now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* ctx) -> CVReturn {
            auto myCtx = reinterpret_cast<CommonRenderingContext *>(ctx);
            if (!myCtx->mFrameScheduled)
            {
                std::shared_ptr<AWindow>* windowSharedPtr = nullptr;
                try {
                    if (auto sharedPtr = aui::ptr::shared_from_this(myCtx->mWindow)) {
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
}

void CommonRenderingContext::destroyNativeWindow(ASurface& window) {
    AUI_ASSERT(mDisplayLink != nullptr);
    CVDisplayLinkStop(static_cast<CVDisplayLinkRef>(mDisplayLink));
    CVDisplayLinkRelease(static_cast<CVDisplayLinkRef>(mDisplayLink));
    mDisplayLink = nullptr;
    [static_cast<NSWindow*>(mWindow->nativeHandle()) close];
}

void CommonRenderingContext::beginPaint(ASurface& window) {
}

void CommonRenderingContext::endPaint(ASurface& window) {
}
