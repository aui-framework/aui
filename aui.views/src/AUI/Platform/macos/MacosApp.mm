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
    assert(("MacosApp should be used only in main thread" && [NSThread isMainThread]));
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
