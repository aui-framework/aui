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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 7/2/2021.
//


#include "AViewProfiler.h"
#include <AUI/Util/kAUI.h>
#include <AUI/Render/RenderHints.h>
#include <glm/ext/matrix_transform.hpp>
#include <AUI/Util/AMetric.h>

void AViewProfiler::displayBoundsOn(const AView& v) {

    RenderHints::PushMatrix m;
    Render::setTransform(glm::translate(glm::mat4(1.f), glm::vec3{v.getPositionInWindow(), 0.f}));
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xff);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    glStencilFunc(GL_EQUAL, 0, 0xff);

    // content
    {
        Render::drawRect(ASolidBrush{ 0x7cb6c180u },
                         { v.getPadding().left, v.getPadding().top },
                         { v.getWidth() - v.getPadding().horizontal(), v.getHeight() - v.getPadding().vertical() });
    }

    // padding
    {
        Render::drawRect(ASolidBrush{ 0xbccf9180u },
                         { 0, 0 },
                         v.getSize());
    }

    // margin
    {
        Render::drawRect(ASolidBrush{ 0xffcca4a0u },
                         { -v.getMargin().left, -v.getMargin().top },
                         { v.getWidth() + v.getMargin().horizontal(), v.getHeight() + v.getMargin().vertical() });
    }

    glDisable(GL_STENCIL_TEST);
    // labels
    {
        int x = -v.getMargin().left;
        int y = v.getHeight() + v.getMargin().bottom + 2_dp;

        FontStyle fs;
        fs.color = 0xffffffffu;
        fs.fontRendering = FontRendering::ANTIALIASING;
        fs.size = 9_pt;

        auto s = Render::prerenderString({ x + 2_dp, y + 1_dp },
                                         v.getCssNames().empty()
                                         ? typeid(v).name()
                                         : v.getCssNames().back() + "\n"_as + AString::number(v.getSize().x) + "x"_as + AString::number(v.getSize().y), fs);

        {
            Render::drawRect(ASolidBrush{ 0x00000070u },
                             { x, y },
                             { s->getWidth() + 4_dp, fs.size * 2.5 + 2_dp });
        }
        s->draw();
    }
}
