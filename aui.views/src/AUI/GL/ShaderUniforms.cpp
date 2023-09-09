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
    gl::Program::Uniform TRANSFORM("SL_uniform_transform");
    gl::Program::Uniform SL_UNIFORM_TRANSFORM("SL_uniform_transform");
    gl::Program::Uniform SIZE("size");
    gl::Program::Uniform MAT("mat");
    gl::Program::Uniform UV_SCALE("uv_scale");
    gl::Program::Uniform SL_UNIFORM_SIGMA("SL_uniform_sigma");
    gl::Program::Uniform SL_UNIFORM_LOWER("SL_uniform_lower");
    gl::Program::Uniform SL_UNIFORM_UPPER("SL_uniform_upper");
    gl::Program::Uniform OUTER_SIZE("SL_uniform_outerSize");
    gl::Program::Uniform INNER_SIZE("SL_uniform_innerSize");
    gl::Program::Uniform OUTER_TO_INNER("SL_uniform_outerToInner");

}