#pragma once

#import <Cocoa/Cocoa.h>

class AWindow;

@interface MainView : NSView<NSTextInputClient>
    - (MainView*) initWithWindow:(AWindow*)window;
@end