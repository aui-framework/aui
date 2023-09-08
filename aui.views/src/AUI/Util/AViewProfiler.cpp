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
    ARender::setTransform(glm::translate(glm::mat4(1.f), glm::vec3{v.getPositionInWindow(), 0.f}));
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xff);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    glStencilFunc(GL_EQUAL, 0, 0xff);

    // content
    {
        ARender::rect(ASolidBrush{0x7cb6c180u},
                      {v.getPadding().left, v.getPadding().top},
                      {v.getWidth() - v.getPadding().horizontal(), v.getHeight() - v.getPadding().vertical()});
    }

    // padding
    {
        ARender::rect(ASolidBrush{0xbccf9180u},
                      {0, 0},
                      v.getSize());
    }

    // margin
    {
        ARender::rect(ASolidBrush{0xffcca4a0u},
                      {-v.getMargin().left, -v.getMargin().top},
                      {v.getWidth() + v.getMargin().horizontal(), v.getHeight() + v.getMargin().vertical()});
    }

    glDisable(GL_STENCIL_TEST);
    // labels
    {
        int x = -v.getMargin().left;
        int y = v.getHeight() + v.getMargin().bottom + 2_dp;

        AFontStyle fs;
        fs.color = 0xffffffffu;
        fs.fontRendering = FontRendering::ANTIALIASING;
        fs.size = 9_pt;

        auto s = ARender::prerenderString({x + 2_dp, y + 1_dp },
                                         v.getAssNames().empty()
                                         ? typeid(v).name()
                                         : *v.getAssNames().begin() + "\n"_as + AString::number(v.getSize().x) + "x"_as + AString::number(v.getSize().y), fs);

        {
            ARender::rect(ASolidBrush{0x00000070u},
                          {x, y},
                          {s->getWidth() + 4_dp, fs.size * 2.5 + 2_dp});
        }
        s->draw();
    }
}
