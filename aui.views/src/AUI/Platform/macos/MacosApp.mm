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
