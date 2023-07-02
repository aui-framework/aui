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