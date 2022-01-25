//
// Created by Alex2772 on 12/9/2021.
//

#include <AUI/Platform/CommonRenderingContext.h>
#include "MacosApp.h"
#import "WindowDelegate.h"
#import "MainView.h"

void CommonRenderingContext::requestFrame() {
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

    auto view = [[MainView alloc] initWithWindow:&window];

    [nsWindow setContentView:view];
    [nsWindow makeFirstResponder:view];
    [nsWindow setDelegate:delegate];
    [nsWindow setAcceptsMouseMovedEvents:YES];
    [nsWindow setRestorable:NO];
    [nsWindow setTitle: [NSString stringWithUTF8String:(init.name.toStdString().c_str())]];
    // Should be retained by window now
    [view release];

    mWindow = &window;


    CVDisplayLinkCreateWithActiveCGDisplays(reinterpret_cast<CVDisplayLinkRef*>(&mDisplayLink));
    CVDisplayLinkSetOutputCallback(reinterpret_cast<CVDisplayLinkRef>(mDisplayLink), [](CVDisplayLinkRef displayLink, const CVTimeStamp* _now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* ctx) -> CVReturn {
        auto myCtx = reinterpret_cast<CommonRenderingContext*>(ctx);
        dispatch_async(dispatch_get_main_queue(), ^{
            myCtx->mWindow->redraw();
            CVDisplayLinkStop(static_cast<CVDisplayLinkRef>(myCtx->mDisplayLink));
        });
        return kCVReturnSuccess;
    }, this);


}

void CommonRenderingContext::destroyNativeWindow(ABaseWindow& window) {
    assert(mDisplayLink != nullptr);
    CVDisplayLinkStop(static_cast<CVDisplayLinkRef>(mDisplayLink));
    CVDisplayLinkRelease(static_cast<CVDisplayLinkRef>(mDisplayLink));
    mDisplayLink = nullptr;
}

void CommonRenderingContext::beginPaint(ABaseWindow& window) {
}

void CommonRenderingContext::endPaint(ABaseWindow& window) {
}
