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
    extern GL::Shader::Uniform COLOR;
    extern GL::Shader::Uniform TRANSFORM;
    extern GL::Shader::Uniform SIZE;
    extern GL::Shader::Uniform MAT;
    extern GL::Shader::Uniform UV_SCALE;
    extern GL::Shader::Uniform SIGMA;
    extern GL::Shader::Uniform LOWER;
    extern GL::Shader::Uniform UPPER;
    extern GL::Shader::Uniform OUTER_SIZE;
    extern GL::Shader::Uniform INNER_SIZE;
    extern GL::Shader::Uniform INNER_TEXEL_SIZE;
    extern GL::Shader::Uniform OUTER_TEXEL_SIZE;
    extern GL::Shader::Uniform OUTER_TO_INNER;

    extern GL::Shader::Uniform COLOR_TL;
    extern GL::Shader::Uniform COLOR_TR;
    extern GL::Shader::Uniform COLOR_BL;
    extern GL::Shader::Uniform COLOR_BR;
}