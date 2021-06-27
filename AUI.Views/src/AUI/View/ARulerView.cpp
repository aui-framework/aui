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
        Render::inst().setTransform(glm::translate(
                    glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3{0, 0, 1.f}),
                    glm::vec3{0, -getWidth(), 0}));
    }

    const int delay1 = 50;
    const int delay2 = delay1 / 2;
    const int delay3 = delay2 / 10;
    Render::inst().setFill(Render::FILL_SOLID);
    {
        RenderHints::PushColor c;
        Render::inst().setColor(getFontStyle().color);
        for (int i = 0; i * delay1 < getLongestSide(); ++i) {
            Render::inst().drawRect(mOffsetPx + operator""_dp(i * delay1), 0.f, 1_dp, getShortestSide());
        }
    }

    // number display
    {
        for (int i = 0; i * delay1 < getLongestSide(); ++i) {
            Render::inst().drawString(mOffsetPx + operator""_dp(i * delay1) + 2_dp,
                                      0.f,
                                      AString::number(i * delay1),
                                      getFontStyle());
        }
    }

    Render::inst().setFill(Render::FILL_SOLID);

    // cursor display
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
        for (int i = 0; i * delay1 < getLongestSide(); ++i) {
            Render::inst().drawRect(mCursorPosPx, 0.f, 1_px, getShortestSide());
        }
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

