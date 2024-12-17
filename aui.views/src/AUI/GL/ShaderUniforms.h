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

#include <AUI/GL/Program.h>

namespace aui::ShaderUniforms {
    extern gl::Program::Uniform COLOR;
    extern gl::Program::Uniform COLOR1;
    extern gl::Program::Uniform COLOR2;
    extern gl::Program::Uniform GRADIENT_MAT_UV;
    extern gl::Program::Uniform TRANSFORM;
    extern gl::Program::Uniform SL_UNIFORM_TRANSFORM;
    extern gl::Program::Uniform UV_SCALE;
    extern gl::Program::Uniform SL_UNIFORM_SIGMA;
    extern gl::Program::Uniform SL_UNIFORM_LOWER;
    extern gl::Program::Uniform SL_UNIFORM_UPPER;
    extern gl::Program::Uniform PIXEL_TO_UV;
    extern gl::Program::Uniform OUTER_SIZE;
    extern gl::Program::Uniform INNER_SIZE;
    extern gl::Program::Uniform INNER_TEXEL_SIZE;
    extern gl::Program::Uniform OUTER_TEXEL_SIZE;
    extern gl::Program::Uniform OUTER_TO_INNER;
    extern gl::Program::Uniform WHICH_ALGO;
    extern gl::Program::Uniform M1;
    extern gl::Program::Uniform M2;
    extern gl::Program::Uniform DIVIDER;
    extern gl::Program::Uniform THRESHOLD;
}
