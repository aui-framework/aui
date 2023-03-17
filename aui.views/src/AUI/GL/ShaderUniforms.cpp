// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "ShaderUniforms.h"

namespace aui::ShaderUniforms {
    gl::Program::Uniform COLOR("color");
    gl::Program::Uniform SL_UNIFORM_COLOR("SL_uniform_color");
    gl::Program::Uniform TRANSFORM("transform");
    gl::Program::Uniform SIZE("size");
    gl::Program::Uniform MAT("mat");
    gl::Program::Uniform UV_SCALE("uv_scale");
    gl::Program::Uniform SIGMA("sigma");
    gl::Program::Uniform LOWER("lower");
    gl::Program::Uniform UPPER("upper");
    gl::Program::Uniform OUTER_SIZE("outerSize");
    gl::Program::Uniform INNER_SIZE("innerSize");
    gl::Program::Uniform INNER_TEXEL_SIZE("innerTexelSize");
    gl::Program::Uniform OUTER_TEXEL_SIZE("outerTexelSize");
    gl::Program::Uniform OUTER_TO_INNER("outer_to_inner");

    gl::Program::Uniform COLOR_TL("color_tl");
    gl::Program::Uniform COLOR_TR("color_tr");
    gl::Program::Uniform COLOR_BL("color_bl");
    gl::Program::Uniform COLOR_BR("color_br");

}