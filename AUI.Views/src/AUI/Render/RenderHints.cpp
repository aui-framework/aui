/**
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#include "RenderHints.h"
#include "AUI/GL/gl.h"


extern unsigned char stencilDepth;

void RenderHints::PushMask::pushMask(const std::function<void()>& maskRenderer) {

    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glStencilOp(GL_KEEP, GL_INCR, GL_INCR);
    glStencilMask(0xff);
    glColorMask(false, false, false, false);

    maskRenderer();

    glColorMask(true, true, true, true);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, ++stencilDepth, 0xff);

}

void RenderHints::PushMask::popMask(const std::function<void()>& maskRenderer) {

    glStencilFunc(GL_ALWAYS, 0, 0xff);
    glStencilOp(GL_KEEP, GL_DECR, GL_DECR);
    glStencilMask(0xff);
    glColorMask(false, false, false, false);

    maskRenderer();

    glColorMask(true, true, true, true);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, --stencilDepth, 0xff);
}

RenderHints::PushMask::Layer::Layer(RenderHints::PushMask::Layer::Direction direction, GLenum strencilComparsion):
    mPrevLayerValue(stencilDepth) {
    stencilDepth += direction;
    glStencilFunc(strencilComparsion, stencilDepth, 0xff);
}

RenderHints::PushMask::Layer::~Layer() {
    glStencilFunc(GL_EQUAL, stencilDepth = mPrevLayerValue, 0xff);
}
