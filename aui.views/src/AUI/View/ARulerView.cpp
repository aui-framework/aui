// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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
    }
}

void ARulerView::render() {
    AView::render();

    if (mLayoutDirection == ALayoutDirection::VERTICAL) {
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
        case ALayoutDirection::VERTICAL:
            return getHeight();
        case ALayoutDirection::HORIZONTAL:
            return getWidth();
    }
    return -1;
}
int ARulerView::getShortestSide() const {
    switch (mLayoutDirection) {
        case ALayoutDirection::VERTICAL:
            return getWidth();
        case ALayoutDirection::HORIZONTAL:
            return getHeight();
    }
    return -1;
}

