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

#include <variant>
#include <AUI/ASS/AShape.h>
#include <AUI/ASS/Property/BackgroundImage.h>   // for Sizing, Repeat, unset_wrap
#include <AUI/Util/Declarative/Modifier.h>
#include <AUI/Common/AOptional.h>
#include <AUI/Render/IRenderer.h>

namespace ass {

/**
 * @brief Image background.
 * @ingroup ass_properties
 * @details
 * Shape-aware equivalent of `BackgroundImage`. Uses C++20 aggregate initialization.
 *
 * ```cpp
 * BackgroundImageV2{ .image = ":icon.svg"_url }
 * BackgroundImageV2{ .image = ":icon.svg"_url, .shape = AShapeRoundedRectangle{ 8_dp } }
 * ```
 */
struct BackgroundImageV2 {
    /**
     * @brief Url to the image.
     * @details
     * In example, ":icon.svg" references to `icon.svg` file in your assets. See AUrl for more info.
     */
    std::variant<AString, _<IDrawable>> image;

    /**
     * @brief Multiply color filter to the image.
     * @details
     * It allows to replace the color of white icons to the specified overlayColor.
     *
     * Gray color multiplied by the specified one gives the darker color.
     *
     * Black color is not affected.
     */
    AColor overlayColor = 0xffffff_rgb;

    /**
     * @brief Repeating. See Repeat
     */
    Repeat repeat = Repeat::NONE;

    /**
     * @brief Sizing. See ass::Sizing
     */
    Sizing sizing = Sizing::FIT_PADDING;

    /**
     * @brief Scale of the image by x and y axis. Default is `{ 1.0, 1.0 }`.
     */
    glm::vec2 scale = glm::vec2{1, 1};

    /**
     * @brief DPI multiplier used to underscale or upperscale the icon.
     * @details
     * In example, you may use 64x64 png icons and set the dpiMargin to 2.0. They will be rendered as 32px icons on
     * 100% scale (instead of 64px), and 64px on 200% scale, remaining crisp. On 300% scale, however, they will be
     * rendered as 96px images, thus becoming blurry, hence usage of SVG icons is recommended.
     */
    float dpiMargin = 1.f;

    ImageRendering imageRendering = ImageRendering::SMOOTH;

    AShape shape = AShapeRectangle {};
};

API_AUI_VIEWS Modifier operator|(Modifier thiz, BackgroundImageV2 value);

}   // namespace ass
