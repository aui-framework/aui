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

#include "ShaderUniforms.h"

namespace aui::ShaderUniforms {
    gl::Program::Uniform COLOR("SL_uniform_color");
    gl::Program::Uniform COLOR1("SL_uniform_color1");
    gl::Program::Uniform COLOR2("SL_uniform_color2");
    gl::Program::Uniform GRADIENT_MAT_UV("SL_uniform_matUv");
    gl::Program::Uniform TRANSFORM("SL_uniform_transform");
    gl::Program::Uniform SL_UNIFORM_TRANSFORM("SL_uniform_transform");
    gl::Program::Uniform UV_SCALE("SL_uniform_uvScale");
    gl::Program::Uniform SL_UNIFORM_SIGMA("SL_uniform_sigma");
    gl::Program::Uniform SL_UNIFORM_LOWER("SL_uniform_lower");
    gl::Program::Uniform SL_UNIFORM_UPPER("SL_uniform_upper");
    gl::Program::Uniform PIXEL_TO_UV("SL_uniform_pixel_to_uv");
    gl::Program::Uniform OUTER_SIZE("SL_uniform_outerSize");
    gl::Program::Uniform INNER_SIZE("SL_uniform_innerSize");
    gl::Program::Uniform OUTER_TO_INNER("SL_uniform_outerToInner");
    gl::Program::Uniform WHICH_ALGO("SL_uniform_whichAlgo");
    gl::Program::Uniform M1("SL_uniform_m1");
    gl::Program::Uniform M2("SL_uniform_m2");
    gl::Program::Uniform DIVIDER("SL_uniform_divider");
    gl::Program::Uniform THRESHOLD("SL_uniform_threshold");
    gl::Program::Uniform KERNEL("SL_uniform_kernel");
}
