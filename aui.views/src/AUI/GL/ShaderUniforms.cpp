/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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