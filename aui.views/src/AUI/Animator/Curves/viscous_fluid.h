//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

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

    float ViscousFluid::operator()(float input) {

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
