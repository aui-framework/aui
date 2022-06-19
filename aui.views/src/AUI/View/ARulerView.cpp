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

//
// Created by alex2 on 6/25/2021.
//


#include "ARulerView.h"
#include <AUI/Platform/AWindow.h>
#include <glm/gtc/matrix_transform.hpp>

ARulerView::ARulerView(LayoutDirection layoutDirection) : mLayoutDirection(layoutDirection) {

    switch (mLayoutDirection) {
        case LayoutDirection::VERTICAL:
            setExpanding({0, 2});
            break;
        case LayoutDirection::HORIZONTAL:
            setExpanding({2, 0});
    }
}

void ARulerView::render() {
    AView::render();

    if (mLayoutDirection == LayoutDirection::VERTICAL) {
        Render::setTransform(glm::translate(
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
        RenderHints::PushColor c;
        Render::setColor(getFontStyle().color);
        for (int i = 0; i * delayLarge < getLongestSide(); ++i) {
            // large dashes
            Render::rect(ASolidBrush{},
                         {mOffsetPx + operator ""_dp(i * delayLarge), 0.f},
                         {1, totalHeight});

            // medium dashes
            Render::rect(ASolidBrush{},
                         {mOffsetPx + operator ""_dp(i * delayLarge + delayMedium), totalHeight / 2},
                         {1, totalHeight / 2});


            // small dashes
            for (int j = 1; j <= 4; ++j) {
                int smallDashOffset = j * delaySmall;
                Render::rect(ASolidBrush{},
                             {mOffsetPx + operator ""_dp(i * delayLarge + smallDashOffset), 3 * totalHeight / 4},
                             {1, totalHeight / 4});

                Render::rect(ASolidBrush{},
                             {mOffsetPx + operator ""_dp(i * delayLarge + smallDashOffset + delayMedium),
                              3 * totalHeight / 4},
                             {1, totalHeight / 4});
            }
        }
    }

    // number display
    {
        for (int i = 0; i * delayLarge < getLongestSide(); ++i) {
            Render::string({mOffsetPx + operator ""_dp(i * delayLarge) + 2_dp, -1.f},
                           AString::number(i * delayLarge),
                           getFontStyle());
        }
    }
}

int ARulerView::getLongestSide() const {
    switch (mLayoutDirection) {
        case LayoutDirection::VERTICAL:
            return getHeight();
        case LayoutDirection::HORIZONTAL:
            return getWidth();
    }
    return -1;
}
int ARulerView::getShortestSide() const {
    switch (mLayoutDirection) {
        case LayoutDirection::VERTICAL:
            return getWidth();
        case LayoutDirection::HORIZONTAL:
            return getHeight();
    }
    return -1;
}

