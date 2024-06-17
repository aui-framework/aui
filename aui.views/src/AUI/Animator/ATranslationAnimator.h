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
// Created by alex2 on 27.08.2020.
//

#pragma once

#include "AAnimator.h"

class API_AUI_VIEWS ATranslationAnimator: public AAnimator {
private:
    glm::vec2 mFrom;
    glm::vec2 mTo;

public:
    ATranslationAnimator(const glm::vec2& from, const glm::vec2& to = {0.f, 0.f});

    void doAnimation(AView* view, float theta) override;
};

