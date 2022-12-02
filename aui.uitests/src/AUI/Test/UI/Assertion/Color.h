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
// Created by Alex2772 on 12/5/2021.
//

#pragma once


#include <AUI/Common/IStringable.h>
#include "AUI/Test/UI/ScreenshotAnalyzer.h"

struct ViewAssertionAverageColor {
    AColor mColor;
    float mInaccuracy;

    ViewAssertionAverageColor(const AColor& color, float inaccuracy = 0.1f) : mColor(color), mInaccuracy(inaccuracy) {}

    bool operator()(const _<AView>& v) {
        auto d = glm::length2(glm::vec3(ScreenshotAnalyzer::makeScreenshot().clip(v).averageColor()) - glm::vec3(mColor)) / 3;
        return d <= mInaccuracy;
    }
};

/**
 * @brief Checks for average color of the element on the screenshot.
 * @param color the color to check for.
 * @param inaccuracy [0.0;1.0) color matching coefficient, where 1 = can be unmatched completely, 0 = should be exactly
 *        the same color.
 */
using averageColor = ViewAssertionAverageColor;