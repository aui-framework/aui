//
// Created by Alexey Titov on 22.01.2022.
//

#include "WindowDelegate.h"

@implementation WindowDelegate {
    AWindow* mWindow;
}
- (WindowDelegate*)initWithWindow:(AWindow*)initWindow {
    mWindow = initWindow;
    return self;
}
@end