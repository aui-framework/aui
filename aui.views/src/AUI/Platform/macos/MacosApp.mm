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
// Created by Alexey Titov on 22.01.2022.
//
#import <AUI/Platform/AWindow.h>
#import <Cocoa/Cocoa.h>
#include "MacosApp.h"

MacosApp& MacosApp::inst() {
    static MacosApp app;
    return app;
}

@interface AUINSApplication: NSApplication {

}
- (id)init;
@end

@implementation AUINSApplication
- (id)init {
    self = [super init];
    return self;
}
@end

MacosApp::MacosApp() {
    AUI_ASSERTX([NSThread isMainThread], "MacosApp should be used only in main thread");
    @autoreleasepool {
        auto nsApp = [AUINSApplication sharedApplication];
        [nsApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        auto mainMenu = [NSMenu new];
        [nsApp setMainMenu:mainMenu];

        auto appMenu = [NSMenu new];
        auto appMenuItem = [NSMenuItem new];
        [appMenu addItemWithTitle: @"About" action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
        [appMenu addItem: [NSMenuItem separatorItem]];
        // [appMenu addItemWithTitle: @"Preferences…" action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@","];
        [appMenu addItemWithTitle: @"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
        [appMenuItem setSubmenu:appMenu];
        [mainMenu addItem:appMenuItem];

        mNsApp = (__bridge void*)nsApp;
    }
}

void MacosApp::run() {
    @autoreleasepool {
        auto app = (__bridge AUINSApplication*)mNsApp;
        [app run];
    }
}
void MacosApp::activateIgnoringOtherApps() {
    auto app = (__bridge AUINSApplication*)mNsApp;
    [app activateIgnoringOtherApps:YES];
    if (auto w = dynamic_cast<AWindow*>(AWindow::current())) {
        [static_cast<NSWindow*>(w->nativeHandle()) makeKeyAndOrderFront:app];
    }
}

void MacosApp::quit() {
    [static_cast<AUINSApplication*>(mNsApp) stop:nil];
    [static_cast<AUINSApplication*>(mNsApp) terminate:nil];
}
