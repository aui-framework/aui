/**
 * =====================================================================================================================
 * Copyright  2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files , to deal in the Software without restriction, including without limitation the
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