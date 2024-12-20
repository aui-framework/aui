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
// Created by alex2 on 18.09.2020.
//

#include "AFocusAnimator.h"
#include <AUI/Render/IRenderer.h>
#include <AUI/View/AView.h>
#include <AUI/Render/RenderHints.h>
#include <AUI/Platform/AWindow.h>

AFocusAnimator::AFocusAnimator() {
}

void AFocusAnimator::doAnimation(AView* view, float theta, IRenderer& render) {
    const float SIZE = 4.f;
    if (theta < 0.99999f) {
        float t = glm::pow(1.f - theta, 4.f);
        render.rectangleBorder(
                ASolidBrush{{0, 0, 0, t}},
                {-t * SIZE, -t * SIZE},
                {t * SIZE * 2 + view->getWidth(), t * SIZE * 2 + view->getHeight()},
                2.f);
    }
}
