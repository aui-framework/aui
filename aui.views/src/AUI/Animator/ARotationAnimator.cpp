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
// Created by alex2772 on 8/20/20.
//

#include "ARotationAnimator.h"
#include "AUI/Render/IRenderer.h"

void ARotationAnimator::doAnimation(AView* view, float theta, IRenderer& render) {
    translateToCenter(view, render);
    render.setTransform(
            glm::rotate(glm::mat4(1.f), glm::mix(mBegin.radians(), mEnd.radians(), theta), glm::vec3{0.f, 0.f, 1.f}));
    translateToCorner(view, render);
}
