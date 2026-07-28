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
// Created by alex2 on 6/25/2021.
//


#include "ARulerView.h"
#include <AUI/Platform/AWindow.h>
#include <glm/gtc/matrix_transform.hpp>

ARulerView::ARulerView(ALayoutDirection layoutDirection) : mLayoutDirection(layoutDirection) {

    switch (mLayoutDirection) {
        case ALayoutDirection::VERTICAL:
            setExpanding({0, 2});
            break;
        case ALayoutDirection::HORIZONTAL:
            setExpanding({2, 0});
        case ALayoutDirection::NONE:
            break;
    }
}

void ARulerView::render(ARenderContext ctx) {
    AView::render(ctx);

    if (mLayoutDirection == ALayoutDirection::VERTICAL) {
        ctx.render.setTransform(glm::translate(
                    glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3{0, 0, 1.f}),
                    glm::vec3{0, -getWidth(), 0}));
    }

    const int delayLarge = 50;
    const int delayMedium = delayLarge / 2;
    const int delaySmall = delayLarge / 10;
    const int totalHeight = getShortestSide();

    /*
     * L
     * L    M
     * LssssMssss
     * 0123456789
     *
     */
    {
        RenderHints::PushColor c(ctx.render);
        ctx.render.setColor(textColor());
        for (int i = 0; i * delayLarge < getLongestSide(); ++i) {
            // large dashes
            ctx.render.rectangle(ASolidBrush{},
                                 {mOffsetPx + operator ""_dp(i * delayLarge), 0.f},
                                 {1, totalHeight});

            // medium dashes
            ctx.render.rectangle(ASolidBrush{},
                                 {mOffsetPx + operator ""_dp(i * delayLarge + delayMedium), totalHeight / 2},
                                 {1, totalHeight / 2});


            // small dashes
            for (int j = 1; j <= 4; ++j) {
                int smallDashOffset = j * delaySmall;
                ctx.render.rectangle(ASolidBrush{},
                                     {mOffsetPx + operator ""_dp(i * delayLarge + smallDashOffset),
                                      3 * totalHeight / 4},
                                     {1, totalHeight / 4});

                ctx.render.rectangle(ASolidBrush{},
                                     {mOffsetPx + operator ""_dp(i * delayLarge + smallDashOffset + delayMedium),
                                      3 * totalHeight / 4},
                                     {1, totalHeight / 4});
            }
        }
    }

    // number display
    {
        for (int i = 0; i * delayLarge < getLongestSide(); ++i) {
            ctx.render.string({mOffsetPx + operator ""_dp(i * delayLarge) + 2_dp, -1.f},
                            AString::number(i * delayLarge),
                            getFontStyle());
        }
    }
}

int ARulerView::getLongestSide() const {
    switch (mLayoutDirection) {
        case ALayoutDirection::VERTICAL:
            return getHeight();
        case ALayoutDirection::HORIZONTAL:
            return getWidth();
        case ALayoutDirection::NONE:
            break;
    }
    return -1;
}
int ARulerView::getShortestSide() const {
    switch (mLayoutDirection) {
        case ALayoutDirection::VERTICAL:
            return getWidth();
        case ALayoutDirection::HORIZONTAL:
            return getHeight();
        case ALayoutDirection::NONE:
            break;
    }
    return -1;
}

void ARulerView::invalidateFont() {

}

