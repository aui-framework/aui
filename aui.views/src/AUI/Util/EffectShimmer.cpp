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
// Created by Alex2772 on 8/31/2022.
//

#include "EffectShimmer.h"
#include "AUI/Render/ARender.h"

void EffectShimmer::draw(AView* view) {
    using namespace std::chrono;

    const float t = (duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() % 1'000) / 1'000.f;

    const auto transparent = AColor(glm::vec4(glm::vec3(mColor), 0.f));

    /*
    ctx.render.rect(ALinearGradientBrush {
        transparent,
        mColor,
        transparent,
        mColor
    }, {(t * 4 - 2) * view->getSize().x, 0}, view->getSize());


    ctx.render.rect(ALinearGradientBrush {
        mColor,
        transparent,
        mColor,
        transparent,
    }, {(t * 4 - 1) * view->getSize().x, 0}, view->getSize());*/

    view->redraw();
}
