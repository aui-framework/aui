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

#include "ShaderUniforms.h"

namespace aui::ShaderUniforms {
    gl::Program::Uniform COLOR("color");
    gl::Program::Uniform ALBEDO("albedo");
    gl::Program::Uniform COLOR1("color1");
    gl::Program::Uniform COLOR2("color2");
    gl::Program::Uniform GRADIENT_MAT_UV("matUv");
    gl::Program::Uniform TRANSFORM("transform");
    gl::Program::Uniform SL_UNIFORM_TRANSFORM("transform");
    gl::Program::Uniform UV_SCALE("uvScale");
    gl::Program::Uniform SL_UNIFORM_SIGMA("sigma");
    gl::Program::Uniform SL_UNIFORM_LOWER("lower");
    gl::Program::Uniform SL_UNIFORM_UPPER("upper");
    gl::Program::Uniform PIXEL_TO_UV("pixel_to_uv");
    gl::Program::Uniform OUTER_SIZE("outerSize");
    gl::Program::Uniform INNER_SIZE("innerSize");
    gl::Program::Uniform WINDOW_SIZE("u_windowSize");
    gl::Program::Uniform MASK("u_mask");
    gl::Program::Uniform MASK_RECT("u_maskRect");
    gl::Program::Uniform USE_MASK("u_useMask");
    gl::Program::Uniform OUTER_TO_INNER("outerToInner");
    gl::Program::Uniform RADIUS("uRadius");
    gl::Program::Uniform RECT_POS("uRectPos");
    gl::Program::Uniform RECT_SIZE("uRectSize");
    gl::Program::Uniform BORDER_WIDTH("uBorderWidth");
    gl::Program::Uniform WHICH_ALGO("whichAlgo");
    gl::Program::Uniform M1("m1");
    gl::Program::Uniform M2("m2");
    gl::Program::Uniform BEGIN("u_begin");
    gl::Program::Uniform END("u_end");
    gl::Program::Uniform DIVIDER("divider");
    gl::Program::Uniform THRESHOLD("threshold");
    gl::Program::Uniform KERNEL("kernel");
    gl::Program::Uniform PREMULTIPLIED("u_premultiplied");
}
