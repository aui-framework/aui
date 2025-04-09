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
// Created by alex2 on 22.09.2020.
//

#include "ASizeAnimator.h"
#include <AUI/View/AView.h>

ASizeAnimator::ASizeAnimator(const glm::ivec2& beginSize, const glm::ivec2& endSize):
    mBeginSize(beginSize), mEndSize(endSize) {

}

void ASizeAnimator::doAnimation(AView* view, float theta, IRenderer& render) {
    if (mEndSize.x == 0 && mEndSize.y == 0) {
        mEndSize = view->getSize();
    }
    auto currentSize = glm::mix(mBeginSize, mEndSize, theta);
    view->setSizeForced(currentSize);
    view->redraw();
}

