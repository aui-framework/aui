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
#include "AUIView.h"

static constexpr auto LOG_TAG = "AUIView";

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

@implementation AUIView {
    AWindow* mAWindow;
}

- (AUIView*)initWithWindow:(AWindow*)window {
    ALOG_DEBUG(LOG_TAG) << "initWithWindow";
    [super init];
    mAWindow = window;
    
    return self;
}

- (BOOL)isOpaque {
    ALOG_DEBUG(LOG_TAG) << "isOpaque";
    return YES;
}

- (BOOL)canBecomeKeyView {
    ALOG_DEBUG(LOG_TAG) << "canBecomeKeyView";
    return YES;
}

- (BOOL)acceptsFirstResponder {
    ALOG_DEBUG(LOG_TAG) << "acceptsFirstResponder";
    return YES;
}

- (BOOL)becomeFirstResponder {
    ALOG_DEBUG(LOG_TAG) << "becomeFirstResponder";
    return YES;
}

- (BOOL)resignFirstResponder {
    ALOG_DEBUG(LOG_TAG) << "resignFirstResponder";
    return YES;
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {
    ALOG_DEBUG(LOG_TAG) << "insertTex";
}

- (void)doCommandBySelector:(SEL)selector {
    ALOG_DEBUG(LOG_TAG) << "doCommandBySelector";
    printf("\n");
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {
    ALOG_DEBUG(LOG_TAG) << "setMarkedText";
}

- (void)unmarkText {
    ALOG_DEBUG(LOG_TAG) << "unmarkText";
    printf("\n");
}

- (NSRange)selectedRange {
    ALOG_DEBUG(LOG_TAG) << "selectedRange";
    return NSMakeRange(0, 0);
}

- (NSRange)markedRange {
    ALOG_DEBUG(LOG_TAG) << "markedRange";
    return NSMakeRange(0, 0);
}

- (BOOL)hasMarkedText {
    ALOG_DEBUG(LOG_TAG) << "hasMarkedText";
    return NO;
}

- (nullable NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    ALOG_DEBUG(LOG_TAG) << "attributedSubstringForProposedRange";
    return nil;
}

- (NSArray<NSAttributedStringKey> *)validAttributesForMarkedText {
    ALOG_DEBUG(LOG_TAG) << "validAttributesForMarkedText";
    // This code is just copied from WebKit except renaming variables.
    static NSArray* const kAttributes = @[
      NSUnderlineStyleAttributeName, NSUnderlineColorAttributeName,
      NSMarkedClauseSegmentAttributeName
    ];
    return kAttributes;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    ALOG_DEBUG(LOG_TAG) << "firstRectForCharacterRange";
    return NSMakeRect(0, 0, 0, 0);
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    ALOG_DEBUG(LOG_TAG) << "characterIndexForPoint";
    return 0;
}

- (void)keyDown:(NSEvent *)event
{
    ALOG_DEBUG(LOG_TAG) << "keyDown";
//    [super keyDown:event];
    printf("\n");
    
    
//    [[self inputContext] handleEvent:event];
    [self interpretKeyEvents:[NSArray arrayWithObject:event]];
}

- (void)keyUp:(NSEvent *)event
{
    ALOG_DEBUG(LOG_TAG) << "keyUp";

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
    onMouseButtonDown(mAWindow, event, AInput::RBUTTON);
}

- (void)rightMouseUp:(NSEvent *)event {
    onMouseButtonUp(mAWindow, event, AInput::RBUTTON);
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
