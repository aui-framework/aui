//
// Created by alex2772 on 1/19/22.
//

#include "UITestState.h"

namespace UITestState {
    bool ourIsUITest = false;
}

bool UITestState::isTesting() {
    return ourIsUITest;
}

void UITestState::beginUITest() {
    ourIsUITest = true;
}
