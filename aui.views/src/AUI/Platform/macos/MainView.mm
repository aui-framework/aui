//
// Created by Alexey Titov on 22.01.2022.
//

#include "MainView.h"

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

@end