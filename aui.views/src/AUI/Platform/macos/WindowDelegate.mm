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

#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/detail/WindowGeometry.h>
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
- (void)windowDidMove:(NSNotification *)notification {
    mWindow->AViewContainer::setPosition(mWindow->getWindowPosition());
}

- (void)windowDidResize:(NSNotification *)notification {
    // keep AUI's idea of the position in sync, otherwise setSize() below would move the window back.
    mWindow->AViewContainer::setPosition(mWindow->getWindowPosition());
    // Real pixels (points * backingScaleFactor), not points * getDpiRatio(): the latter also includes
    // ASurface::ScalingParams (UI scale), which affects dp<->px conversion only. Mixing them made the window (and the
    // GL framebuffer) larger than the actual drawable.
    const float s = [static_cast<NSWindow*>(mWindow->nativeHandle()) backingScaleFactor];
    const NSRect contentFrame = [[static_cast<NSWindow*>(mWindow->nativeHandle()) contentView] frame];
    mWindow->getRenderingContext()->beginResize(*mWindow);
    mWindow->onResize(aui::platform::detail::pointsToPixels(contentFrame.size.width, s),
                      aui::platform::detail::pointsToPixels(contentFrame.size.height, s));
    mWindow->getRenderingContext()->endResize(*mWindow);
    mWindow->redraw();
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    mWindow->onCloseButtonClicked();
    return FALSE;
}

@end
