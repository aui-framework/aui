/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2026 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <gtest/gtest.h>
#include <AUI/Test/UI/ScreenshotAnalyzer.h>
#include <AUI/Test/UI/UITestUtil.h>

namespace uitest_util {

/**
 * @brief Empty pixel rows above and below the dark text of a view on a white background.
 */
struct InkGaps {
    int top;
    int bottom;

    [[nodiscard]]
    int difference() const noexcept {
        return top - bottom;
    }
};

/**
 * @brief Takes a screenshot of the view and measures the empty space above and below the dark pixels inside of it.
 * @details
 * The view is expected to be already shown, laid out and drawn on a white background.
 */
inline InkGaps measureInkGaps(const _<AView>& view) {
    uitest::frame();
    auto shot = ScreenshotAnalyzer::makeScreenshot().clip(view);
    const int height = int(shot.image().height());
    int first = -1;
    int last = -1;
    for (int y = 0; y < height; ++y) {
        const auto row = shot.clip({ 0, y }, { shot.image().width(), 1 });
        if (glm::vec3(row.averageColor()).r < 0.97f) {
            if (first < 0) {
                first = y;
            }
            last = y;
        }
    }
    EXPECT_GE(first, 0) << "no text rendered";
    return { first, height - 1 - last };
}

}   // namespace uitest_util
