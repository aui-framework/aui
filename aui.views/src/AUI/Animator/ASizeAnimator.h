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
// Created by alex2 on 27.08.2020.
//

#pragma once

#include <AUI/Render/IRenderer.h>
#include "AAnimator.h"

class API_AUI_VIEWS ASizeAnimator: public AAnimator {
private:
    glm::ivec2 mBeginSize;
    glm::ivec2 mEndSize;

public:
    ASizeAnimator(const glm::ivec2& beginSize = {0, 0}, const glm::ivec2& endSize = {0, 0});

    void doAnimation(AView* view, float theta, IRenderer& render) override;
};

