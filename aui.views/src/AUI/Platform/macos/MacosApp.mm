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
    auto pool = [[NSAutoreleasePool alloc] init];
    auto nsApp = [AUINSApplication sharedApplication];
    //[NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    auto menuBar = [[NSMenu alloc] initWithTitle:(@"AUI Application")];
    [nsApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [nsApp setMainMenu:menuBar];

    [menuBar release];
    [pool release];
    mNsApp = nsApp;
}

void MacosApp::run() {
    [static_cast<AUINSApplication*>(mNsApp) run];
}
void MacosApp::activateIgnoringOtherApps() {
    auto app = static_cast<AUINSApplication*>(mNsApp);
    [app activateIgnoringOtherApps:YES];
    if (auto w = dynamic_cast<AWindow*>(AWindow::current())) {
        [static_cast<NSWindow*>(w->nativeHandle()) makeKeyAndOrderFront:app];
    }
}

void MacosApp::quit() {
    [static_cast<AUINSApplication*>(mNsApp) stop:nil];
    [static_cast<AUINSApplication*>(mNsApp) terminate:nil];
}
