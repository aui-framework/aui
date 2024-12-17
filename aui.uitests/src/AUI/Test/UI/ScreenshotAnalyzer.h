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

#pragma once

#include <AUI/Image/AImage.h>
#include "AUI/Common/AColor.h"
#include "AUI/View/AView.h"

class API_AUI_UITESTS ScreenshotAnalyzer {
public:
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


