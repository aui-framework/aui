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

#pragma once

#include <AUI/Image/AImage.h>
#include "AUI/Common/AColor.h"
#include "AUI/View/AView.h"

/**
 * @brief A screenshot of the current window contents which can be analyzed in tests.
 * @ingroup uitests
 * @details
 * Most of the time, you don't need this class directly; `averageColor` and `pixelColorAt` assertions use it. Use it when
 * you need something custom:
 *
 * ```cpp
 * auto screenshot = ScreenshotAnalyzer::makeScreenshot().clip(By::name("#preview").one());
 * EXPECT_EQ(screenshot.image().size(), glm::uvec2(64, 64));
 * ```
 *
 * See [uitests-screenshots].
 */
class API_AUI_UITESTS ScreenshotAnalyzer {
public:
    /**
     * @brief Takes a screenshot of the current window.
     */
    static ScreenshotAnalyzer makeScreenshot();

    [[nodiscard]]
    AColor averageColor() const noexcept {
        return mImage.averageColor();
    }

    ScreenshotAnalyzer clip(const _<AView>& view) const {
        return clip(view->getPositionInWindow(), view->getSize());
    }
    ScreenshotAnalyzer clip(glm::uvec2 position, glm::uvec2 size) const {
        position = glm::max(position, {0, 0});
        return mImage.cropped(position, glm::clamp(size, {0, 0}, mImage.size() - position));
    }

    const AImage& image() const {
        return mImage;
    }

private:
    AImage mImage;

    ScreenshotAnalyzer(AImage image) : mImage(std::move(image)) {}
};


