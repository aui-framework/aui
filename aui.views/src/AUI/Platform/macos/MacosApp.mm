//
// Created by Alexey Titov on 22.01.2022.
//
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
