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

#include <AUI/GL/Program.h>

namespace aui::ShaderUniforms {
    extern gl::Program::Uniform COLOR;
    extern gl::Program::Uniform SL_UNIFORM_COLOR;
    extern gl::Program::Uniform TRANSFORM;
    extern gl::Program::Uniform SIZE;
    extern gl::Program::Uniform MAT;
    extern gl::Program::Uniform UV_SCALE;
    extern gl::Program::Uniform SIGMA;
    extern gl::Program::Uniform LOWER;
    extern gl::Program::Uniform UPPER;
    extern gl::Program::Uniform OUTER_SIZE;
    extern gl::Program::Uniform INNER_SIZE;
    extern gl::Program::Uniform INNER_TEXEL_SIZE;
    extern gl::Program::Uniform OUTER_TEXEL_SIZE;
    extern gl::Program::Uniform OUTER_TO_INNER;

    extern gl::Program::Uniform COLOR_TL;
    extern gl::Program::Uniform COLOR_TR;
    extern gl::Program::Uniform COLOR_BL;
    extern gl::Program::Uniform COLOR_BR;
}