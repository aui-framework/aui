// AUI Framework - Declarative UI toolkit for modern C++17
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
    gl::Shader::Uniform COLOR("color");
    gl::Shader::Uniform TRANSFORM("transform");
    gl::Shader::Uniform SIZE("size");
    gl::Shader::Uniform MAT("mat");
    gl::Shader::Uniform UV_SCALE("uv_scale");
    gl::Shader::Uniform SIGMA("sigma");
    gl::Shader::Uniform LOWER("lower");
    gl::Shader::Uniform UPPER("upper");
    gl::Shader::Uniform OUTER_SIZE("outerSize");
    gl::Shader::Uniform INNER_SIZE("innerSize");
    gl::Shader::Uniform INNER_TEXEL_SIZE("innerTexelSize");
    gl::Shader::Uniform OUTER_TEXEL_SIZE("outerTexelSize");
    gl::Shader::Uniform OUTER_TO_INNER("outer_to_inner");

    gl::Shader::Uniform COLOR_TL("color_tl");
    gl::Shader::Uniform COLOR_TR("color_tr");
    gl::Shader::Uniform COLOR_BL("color_bl");
    gl::Shader::Uniform COLOR_BR("color_br");

}