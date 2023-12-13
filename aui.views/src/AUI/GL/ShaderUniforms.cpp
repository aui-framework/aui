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
    gl::Program::Uniform PIXEL_TO_UV("SL_uniform_pixelToUv");
    gl::Program::Uniform OUTER_SIZE("SL_uniform_outerSize");
    gl::Program::Uniform INNER_SIZE("SL_uniform_innerSize");
    gl::Program::Uniform OUTER_TO_INNER("SL_uniform_outerToInner");
    gl::Program::Uniform WHICH_ALGO("SL_uniform_whichAlgo");
    gl::Program::Uniform M1("SL_uniform_m1");
    gl::Program::Uniform M2("SL_uniform_m2");
}
