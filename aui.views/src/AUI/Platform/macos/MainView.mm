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
    window->onMouseMove(pos(window, event));
}

void onMouseButtonDown(AWindow* window, NSEvent* event, AInput::Key key) {
    window->onMousePressed(pos(window, event), key);
}

void onMouseButtonUp(AWindow* window, NSEvent* event, AInput::Key key) {
    window->onMouseReleased(pos(window, event), key);
}

@implementation MainView {
    AWindow* mAWindow;
}

- (MainView*)initWithWindow:(AWindow*)window {
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
    onMouseButtonDown(mAWindow, event, AInput::LButton);
}

- (void)mouseUp:(NSEvent *)event {
    onMouseButtonUp(mAWindow, event, AInput::LButton);
}

- (void)rightMouseDown:(NSEvent *)event {
    onMouseButtonDown(mAWindow, event, AInput::LButton);
}

- (void)rightMouseUp:(NSEvent *)event {
    onMouseButtonUp(mAWindow, event, AInput::LButton);
}

@end