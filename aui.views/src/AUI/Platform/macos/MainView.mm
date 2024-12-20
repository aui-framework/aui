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

#include <AUI/Platform/AWindow.h>
#include "MainView.h"

glm::ivec2 pos(AWindow* window, NSEvent* event) {
    float s = window->getDpiRatio();
    const NSPoint pos = [event locationInWindow];
    auto size = [[[event window] contentView] frame].size;
    return glm::ivec2{pos.x * s, (size.height - pos.y) * s};
}

void onMouseMoved(AWindow* window, NSEvent* event) {
    window->onPointerMove(pos(window, event), {});
}

void onMouseButtonDown(AWindow* window, NSEvent* event, AInput::Key key) {
    window->onPointerPressed({pos(window, event), APointerIndex::button(key)});
}

void onMouseButtonUp(AWindow* window, NSEvent* event, AInput::Key key) {
    window->onPointerReleased({pos(window, event), APointerIndex::button(key)});
}

@implementation MainView {
    AWindow* mAWindow;
}

- (MainView*)initWithWindow:(AWindow*)window {
    [super init];
    mAWindow = window;
    
    return self;
}

- (BOOL)isOpaque {
    return YES;
}

- (BOOL)canBecomeKeyView {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {

}

- (void)doCommandBySelector:(SEL)selector {

}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {

}

- (void)unmarkText {

}

- (NSRange)selectedRange {
    return NSRange();
}

- (NSRange)markedRange {
    return NSRange();
}

- (BOOL)hasMarkedText {
    return NO;
}

- (nullable NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    return nil;
}

- (NSArray<NSAttributedStringKey> *)validAttributesForMarkedText {
    return nil;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    return NSRect();
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    return 0;
}
- (void)mouseMoved:(NSEvent *)event {
    onMouseMoved(mAWindow, event);
}

- (void)mouseDragged:(NSEvent *)event {
    onMouseMoved(mAWindow, event);
}

- (void)rightMouseDragged:(NSEvent *)event {
    onMouseMoved(mAWindow, event);
}

- (void)otherMouseDragged:(NSEvent *)event {
    onMouseMoved(mAWindow, event);
}

- (void)mouseDown:(NSEvent *)event {
    onMouseButtonDown(mAWindow, event, AInput::LBUTTON);
}

- (void)mouseUp:(NSEvent *)event {
    onMouseButtonUp(mAWindow, event, AInput::LBUTTON);
}

- (void)rightMouseDown:(NSEvent *)event {
    onMouseButtonDown(mAWindow, event, AInput::LBUTTON);
}

- (void)rightMouseUp:(NSEvent *)event {
    onMouseButtonUp(mAWindow, event, AInput::LBUTTON);
}

- (void)scrollWheel:(NSEvent *)event {
    CGFloat deltaX;
    CGFloat deltaY;
    if ([event hasPreciseScrollingDeltas]) {
        deltaX = [event scrollingDeltaX];
        deltaY = [event scrollingDeltaY];
    } else {
        // https://github.com/chromium/chromium/blob/2dc93b871d2b02f895ada7f1a6fbb642cb6ec9da/ui/events/cocoa/events_mac.mm#L138-L157
        deltaX = [event deltaX] * 40.0;
        deltaY = [event deltaY] * 40.0;
    }
    CGFloat scale = mAWindow->getDpiRatio();
    deltaX *= scale;
    deltaY *= scale;
    mAWindow->onScroll({
        .origin = pos(mAWindow, event),
        .delta = glm::ivec2{ -deltaX, -deltaY }
    });
}
@end
