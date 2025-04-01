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

#pragma once

#include <glm/glm.hpp>

namespace aui::animation_curves {
    class ViscousFluid {
    public:
        float operator()(float input);

    private:
        // controls the viscous fluid effect (how much of it).
        constexpr static float VISCOUS_FLUID_SCALE = 8.0f;

        constexpr static float viscousFluid(float x) {
            x *= VISCOUS_FLUID_SCALE;
            if (x < 1.0f) {
                x -= (1.0f - (float)glm::exp(-x));
            } else {
                float start = 0.36787944117f;   // 1/e == exp(-1)
                x = 1.0f - (float)glm::exp(1.0f - x);
                x = start + x * (1.0f - start);
            }
            return x;
        }
    };

    inline float ViscousFluid::operator()(float input) {

        // must be set to 1.0 (used in viscousFluid())
        const float VISCOUS_FLUID_NORMALIZE = 1.0f / viscousFluid(1.0f);

        // account for very small floating-point error
        const float VISCOUS_FLUID_OFFSET = 1.0f - VISCOUS_FLUID_NORMALIZE * viscousFluid(1.0f);

        float interpolated = VISCOUS_FLUID_NORMALIZE * viscousFluid(input);
        if (interpolated > 0) {
            return interpolated + VISCOUS_FLUID_OFFSET;
        }
        return interpolated;
    }
}
