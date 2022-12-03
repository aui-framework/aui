// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#include <AUI/GL/Shader.h>

namespace aui::ShaderUniforms {
    extern gl::Shader::Uniform COLOR;
    extern gl::Shader::Uniform TRANSFORM;
    extern gl::Shader::Uniform SIZE;
    extern gl::Shader::Uniform MAT;
    extern gl::Shader::Uniform UV_SCALE;
    extern gl::Shader::Uniform SIGMA;
    extern gl::Shader::Uniform LOWER;
    extern gl::Shader::Uniform UPPER;
    extern gl::Shader::Uniform OUTER_SIZE;
    extern gl::Shader::Uniform INNER_SIZE;
    extern gl::Shader::Uniform INNER_TEXEL_SIZE;
    extern gl::Shader::Uniform OUTER_TEXEL_SIZE;
    extern gl::Shader::Uniform OUTER_TO_INNER;

    extern gl::Shader::Uniform COLOR_TL;
    extern gl::Shader::Uniform COLOR_TR;
    extern gl::Shader::Uniform COLOR_BL;
    extern gl::Shader::Uniform COLOR_BR;
}