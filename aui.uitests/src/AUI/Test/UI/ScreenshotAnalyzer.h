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
#include "AUI/Platform/AWindow.h"

class API_AUI_UITESTS ScreenshotAnalyzer {
public:
    static ScreenshotAnalyzer makeScreenshot();

    [[nodiscard]]
    AColor averageColor() const noexcept {
        return mImage.averageColor();
    }

    ScreenshotAnalyzer clip(const _<AView>& view) const {
        const auto windowSize = AWindow::current()->getSize();
        const glm::dvec2 scale = (windowSize.x == 0 || windowSize.y == 0)
                ? glm::dvec2(1.0)
                : glm::dvec2(mImage.size()) / glm::dvec2(windowSize);
        return clip(glm::uvec2(glm::dvec2(view->getPositionInWindow()) * scale),
                    glm::uvec2(glm::round(glm::dvec2(view->getSize()) * scale)));
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


