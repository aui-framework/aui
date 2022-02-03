//
// Created by Alexey Titov on 22.01.2022.
//

#include <AUI/Platform/AWindow.h>
#include "WindowDelegate.h"

@implementation WindowDelegate {
    AWindow* mWindow;
}
- (WindowDelegate*)initWithWindow:(AWindow*)initWindow {
    mWindow = initWindow;
    return self;
}

/*
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize {
    float s = mWindow->getDpiRatio();
    const CGSize contentFrame = [[static_cast<NSWindow*>(mWindow->nativeHandle()) contentView] frame].size;
    mWindow->getRenderingContext()->beginResize(*mWindow);
    mWindow->AViewContainer::setSize(contentFrame.width * s, contentFrame.height * s);
    mWindow->getRenderingContext()->endResize(*mWindow);
    return frameSize;
}
*/
- (void)windowDidResize:(NSNotification *)notification {
    float s = mWindow->getDpiRatio();
    const NSRect contentFrame = [[static_cast<NSWindow*>(mWindow->nativeHandle()) contentView] frame];
    mWindow->getRenderingContext()->beginResize(*mWindow);
    mWindow->AViewContainer::setSize(contentFrame.size.width * s, contentFrame.size.height * s);
    mWindow->getRenderingContext()->endResize(*mWindow);
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    mWindow->onCloseButtonClicked();
    return FALSE;
}

@end