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

extern bool gKeyStates[AInput::KEYCOUNT + 1];

namespace {

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

bool isEventReserved(NSEvent* event) {
    return false; // stub
}
}

@implementation AUIView {
    AWindow* mAWindow;

    BOOL _hasMarkedText;
    int _currentKeyDownCode;

  // The set of key codes from key down events that we haven't seen the matching
  // key up events yet.
  // Used for filtering out non-matching NSEventTypeKeyUp events.
  std::set<unsigned short> mUnmatchedKeyDownCodes;
}

- (AUIView*)initWithWindow:(AWindow*)window {
    ALOG_DEBUG(LOG_TAG) << "initWithWindow";
    [super init];
    mAWindow = window;

    _hasMarkedText = NO;
    _currentKeyDownCode = 0;
    
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

- (BOOL)performKeyEquivalent:(NSEvent*)theEvent {
    ALOG_DEBUG(LOG_TAG) << "performKeyEquivalent";
    // Command key combinations are sent via performKeyEquivalent rather than
    // keyDown:. We just forward this.
    [self keyEvent:theEvent wasKeyEquivalent:YES];
    return YES;
}

- (BOOL)_wantsKeyDownForEvent:(NSEvent*)event {
  // This is a SPI that AppKit apparently calls after |performKeyEquivalent:|
  // returned NO. If this function returns |YES|, Cocoa sends the event to
  // |keyDown:| instead of doing other things with it. Ctrl-tab will be sent
  // to us instead of doing key view loop control, ctrl-left/right get handled
  // correctly, etc.
  // (However, there are still some keys that Cocoa swallows, e.g. the key
  // equivalent that Cocoa uses for toggling the input language. In this case,
  // that's actually a good thing, though -- see http://crbug.com/26115 .)
  return YES;
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {
    ALOG_DEBUG(LOG_TAG) << "insertText";
    // Could be NSString or NSAttributedString, so we have
    // to test and convert.
    AString str = [&]{
        if ([string isKindOfClass:[NSAttributedString class]]) {
            return [[string string] UTF8String];
        } else {
            return [string UTF8String];
        }
    }();
    
    for (const auto c : str) {
        mAWindow->onCharEntered(c);
    }
}

- (void)doCommandBySelector:(SEL)selector {
    ALOG_DEBUG(LOG_TAG) << "doCommandBySelector";
}

- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {
    ALOG_DEBUG(LOG_TAG) << "setMarkedText";
}

- (void)unmarkText {
    ALOG_DEBUG(LOG_TAG) << "unmarkText";
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
    // An input method calls this function to figure out whether or not an
  // application is really composing a text. If it is composing, it calls
  // the markedRange method, and maybe calls the setMarkedText method.
  // It seems an input method usually calls this function when it is about to
  // cancel an ongoing composition. If an application has a non-empty marked
  // range, it calls the setMarkedText method to delete the range.
  return _hasMarkedText;
}

- (nullable NSAttributedString *)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    ALOG_DEBUG(LOG_TAG) << "attributedSubstringForProposedRange";
    return nil;
}

- (NSArray<NSAttributedStringKey> *)validAttributesForMarkedText {
    ALOG_DEBUG(LOG_TAG) << "validAttributesForMarkedText";
    // This code is just copied from WebKit except renaming variables.
    static auto array = @[
      NSUnderlineStyleAttributeName, NSUnderlineColorAttributeName,
      NSMarkedClauseSegmentAttributeName
    ];
    [array retain]; // arc would have destroyed array without this; despite static storage
    return array;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    ALOG_DEBUG(LOG_TAG) << "firstRectForCharacterRange";
    return NSMakeRect(0, 0, 0, 0);
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    ALOG_DEBUG(LOG_TAG) << "characterIndexForPoint";
    return 0;
}

- (void)keyEvent:(NSEvent*)theEvent wasKeyEquivalent:(BOOL)equiv {
    ALOG_DEBUG(LOG_TAG) << "keyEvent";
    NSEventType eventType = [theEvent type];
    NSEventModifierFlags modifierFlags = [theEvent modifierFlags];
    int keyCode = [theEvent keyCode];

    // If the user changes the system hotkey mapping after Chrome has been
    // launched, then it is possible that a formerly reserved system hotkey is no
    // longer reserved. The hotkey would have skipped the renderer, but would
    // also have not been handled by the system. If this is the case, immediately
    // return.
    if (isEventReserved(theEvent))
      return;

    if (eventType == NSEventTypeFlagsChanged) {
      // Ignore NSEventTypeFlagsChanged events from the NumLock and Fn keys as
      // Safari does in -[WebHTMLView flagsChanged:] (of "WebHTMLView.mm").
      // Also ignore unsupported |keyCode| (255) generated by Convert, NonConvert
      // and KanaMode from JIS PC keyboard.
      if (!keyCode || keyCode == 10 || keyCode == 63 || keyCode == 255)
        return;
    }

    // Don't cancel child popups; the key events are probably what's triggering
    // the popup in the first place.

    // Do not forward key up events unless preceded by a matching key down.
    if (eventType == NSEventTypeKeyUp) {
      auto numErased = mUnmatchedKeyDownCodes.erase(keyCode);
      if (numErased < 1)
        return;
    }

    bool shouldAutohideCursor = eventType == NSEventTypeKeyDown &&
                                !(modifierFlags & NSEventModifierFlagCommand);

    // We only handle key down events and just simply forward other events.
    if (eventType != NSEventTypeKeyDown) {
      //_hostHelper->ForwardKeyboardEvent(event, latencyInfo);
        auto key = AInput::fromNative(keyCode);
        gKeyStates[key] = false;
        mAWindow->onKeyUp(key);

      // Possibly autohide the cursor.
      if (shouldAutohideCursor) {
        [NSCursor setHiddenUntilMouseMoves:YES];
      }
      return;
    }

    mUnmatchedKeyDownCodes.insert(keyCode);

    // Records the current marked text state, so that we can know if the marked
    // text was deleted or not after handling the key down event.
    BOOL oldHasMarkedText = _hasMarkedText;

    // Tells insertText: and doCommandBySelector: that we are handling a key
    // down event.
    _currentKeyDownCode = keyCode;

    // These variables might be set when handling the keyboard event.
    // Clear them here so that we can know whether they have changed afterwards.
    /*
    _textToBeInserted.clear();
    _markedText.clear();
    _markedTextSelectedRange = NSMakeRange(NSNotFound, 0);
    _imeTextSpans.clear();
    _setMarkedTextReplacementRange = gfx::Range::InvalidRange();
    _unmarkTextCalled = NO;
    _hasEditCommands = NO;
    _editCommands.clear();*/
    [self interpretKeyEvents:@[ theEvent ]];

    // Possibly autohide the cursor.
    if (shouldAutohideCursor) {
      [NSCursor setHiddenUntilMouseMoves:YES];
    }


    auto key = AInput::fromNative(keyCode);
    gKeyStates[key] = true;
    mAWindow->onKeyDown(AInput::fromNative(keyCode));
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
