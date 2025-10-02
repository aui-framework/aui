/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2 on 7/2/2021.
//


#include "AViewProfiler.h"
#include <AUI/Util/kAUI.h>
#include <AUI/Render/RenderHints.h>
#include <glm/ext/matrix_transform.hpp>
#include <AUI/Util/AMetric.h>

void AViewProfiler::displayBoundsOn(AView& v, ARenderContext ctx) {

    RenderHints::PushMatrix m(ctx.render);
    ctx.render.setTransform(glm::translate(glm::mat4(1.f), glm::vec3{v.getPositionInWindow(), 0.f}));
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xff);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);
    glStencilFunc(GL_EQUAL, 0, 0xff);

    // content
    {
        ctx.render.rectangle(ASolidBrush{0x7cb6c180u},
                             {v.getPadding().left, v.getPadding().top},
                             {v.getWidth() - v.getPadding().horizontal(), v.getHeight() - v.getPadding().vertical()});
    }

    // padding
    {
        ctx.render.rectangle(ASolidBrush{0xbccf9180u},
                             {0, 0},
                             v.getSize());
    }

    // margin
    {
        ctx.render.rectangle(ASolidBrush{0xffcca4a0u},
                             {-v.getMargin().left, -v.getMargin().top},
                             {v.getWidth() + v.getMargin().horizontal(), v.getHeight() + v.getMargin().vertical()});
    }

    glDisable(GL_STENCIL_TEST);
    // labels
    {
        int x = -v.getMargin().left;
        int y = v.getHeight() + v.getMargin().bottom + 2_dp;

        AFontStyle fs {
            .size = static_cast<unsigned int>(9_pt),
            .fontRendering = FontRendering::ANTIALIASING,
        };

        auto s = ctx.render.prerenderString({x + 2_dp, y + 1_dp + fs.getAscenderHeight() },
                                         v.getAssNames().empty()
                                         ? typeid(v).name()
                                         : *v.getAssNames().begin() + "\n"_as + AString::number(v.getSize().x) + "x"_as + AString::number(v.getSize().y), fs);

        {
            ctx.render.rectangle(ASolidBrush{0x00000070u},
                                 {x, y},
                                 {s->getWidth() + 4_dp, fs.size * 2.5 + 2_dp});
        }
        s->draw();
    }
}
