#pragma once
#import <Cocoa/Cocoa.h>

class AWindow;

@interface WindowDelegate : NSObject<NSWindowDelegate>
- (WindowDelegate*)initWithWindow:(AWindow*)initWindow;
@end