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
#include <AUI/Platform/AApplication.h>

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

@interface AUIAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AUIAppDelegate
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    // lifetime is controlled by AApplication (see AWindow::quit)
    return NO;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
    // Cmd+Q / Dock "Quit": never let Cocoa call exit() - return to aui_main so cleanup and destructors are performed.
    AApplication::inst().quit();
    [sender stop:nil];
    // post a dummy event so [NSApp run] notices the stop request
    NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:NSZeroPoint modifierFlags:0
                                       timestamp:0 windowNumber:0 context:nil subtype:0 data1:0 data2:0];
    [sender postEvent:event atStart:YES];
    return NSTerminateCancel;
}

- (BOOL)applicationShouldHandleReopen:(NSApplication*)sender hasVisibleWindows:(BOOL)flag {
    // click on Dock icon
    AActivation activation = aui::detail::single_instance::makeCurrentActivation();
    AUI_EMIT_FOREIGN(&AApplication::inst(), activated, std::move(activation));
    return YES;
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
        static AUIAppDelegate* delegate = [AUIAppDelegate new];
        [nsApp setDelegate:delegate];
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
// don't call terminate: here, it calls exit() skipping AUI cleanup. stopping the run loop returns control to
    // aui_main.
    auto app = static_cast<AUINSApplication*>(mNsApp);
    [app stop:nil];
    NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined location:NSZeroPoint modifierFlags:0
                                       timestamp:0 windowNumber:0 context:nil subtype:0 data1:0 data2:0];
    [app postEvent:event atStart:YES
    [static_cast<AUINSApplication*>(mNsApp) stop:nil];
    [static_cast<AUINSApplication*>(mNsApp) terminate:nil];
}
