/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by Alex2772 on 12/5/2021.
//

#pragma once


#include <AUI/Common/IStringable.h>
#include "AUI/Test/UI/ScreenshotAnalyzer.h"

struct ViewAssertionAverageColor {
    AColor mColor;
    aui::float_within_0_1 mInaccuracy;

    ViewAssertionAverageColor(const AColor& color, aui::float_within_0_1 inaccuracy = 0.1f) : mColor(color), mInaccuracy(inaccuracy) {}

    bool operator()(const _<AView>& v) {
        glm::vec3 average(ScreenshotAnalyzer::makeScreenshot().clip(v).averageColor());
        auto d = glm::length2(average - glm::vec3(mColor)) / 3;
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

struct ViewAssertionPixelColorAt {
    using RelativePosition = glm::vec<2, aui::float_within_0_1>;
    RelativePosition mRelativePosition;
    AColor mColor;
    aui::float_within_0_1 mInaccuracy;

    ViewAssertionPixelColorAt(RelativePosition relativePosition, const AColor& color, aui::float_within_0_1 inaccuracy = 0.1f): mRelativePosition(relativePosition), mColor(color), mInaccuracy(inaccuracy) {}

    bool operator()(const _<AView>& v) {
        auto screenshot = ScreenshotAnalyzer::makeScreenshot().clip(v);
        auto mappedPos = glm::uvec2(glm::vec2(screenshot.image().size() - 1u) * glm::vec2(mRelativePosition));
        auto target = glm::vec3(screenshot.image().get(mappedPos));
        auto d = glm::length2(target - glm::vec3(mColor)) / 3;
        return d <= mInaccuracy;
    }
};

/**
 * @brief Checks for pixel color on view on the screenshot.
 * @param relativePosition a vec2 within [0.0;1.0)
 * @param color the color to check for.
 * @param inaccuracy [0.0;1.0) color matching coefficient, where 1 = can be unmatched completely, 0 = should be exactly
 *        the same color.
 * @details
 * relativePosition is a coordinate within the matched view(s) coordinate space, multiplied by its size. For example:
 *
 * - (0,0) corresponds to the top left corner.
 * - (1,1) corresponds to the bottom right corner.
 * - (0.5,0.5) corresponds to the center.
 */
using pixelColorAt = ViewAssertionPixelColorAt;
