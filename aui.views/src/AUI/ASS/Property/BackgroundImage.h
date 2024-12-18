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
// Created by alex2 on 3.12.2020.
//

#pragma once

#include <AUI/Render/IRenderer.h>
#include <AUI/Common/IStringable.h>
#include "IProperty.h"

namespace ass {
    /**
     * @brief Determines sizing rules of a background texture (BackgroundImage).
     * @ingroup ass
     */
    enum class Sizing {
        /**
         * @brief Image is kept in it's original size.
         */
        NONE,

        /**
         * @brief Resize image to view's area without keeping aspect ratio.
         */
        FIT,

        /**
         * @brief Center the image.
         */
        CENTER,

        /**
         * @brief Tile texture. Use with Repeat::X_y.
         */
        TILE,

        /**
         * @brief Resize image to view's content area without keeping aspect ratio.
         */
        FIT_PADDING,

        /**
         * @brief Resize image to view's area keeping aspect ratio and cutting of excess parts. Matches CSS
         * background-size: cover
         */
        COVER,

        /**
         * @brief Resize image to view's area keeping aspect ratio and keeping space not covered by the image. Matches
         * CSS background-size: contain
         */
        CONTAIN,

        /**
         * @brief Resize image to view's content area keeping aspect ratio and keeping space not covered by the image.
         * Partially matches CSS background-size: contain
         */
        CONTAIN_PADDING,

        /**
         * @brief Texture divided by 4 parts of the same size, keeping their original size. Useful for textured buttons
         * and inputs in games.
         * @details When the view is larger than the texture, the free space is covered by stretching the central row
         * (for vertical) and the central column (for horizontal).
         */
        SPLIT_2X2,

        /**
         * @brief Texture is cropped by BackgroundCropping rule.
         */
        CROPPED,
    };

    /**
     * @brief Represents textured (image) background.
     * @ingroup ass
     */
    struct BackgroundImage {
        /**
         * @brief Url to the image.
         * @details
         * In example, ":icon.svg" references to `icon.svg` file in your assets. See AUrl for more info.
         */
        unset_wrap<AString> url;

        /**
         * @brief Multiply color filter to the image.
         * @details
         * It allows to replace the color of white icons to the specified overlayColor.
         *
         * Gray color multiplied by the specified one gives the darker color.
         *
         * Black color is not affected.
         */
        unset_wrap<AColor> overlayColor;

        /**
         * @brief Repeating. See Repeat
         */
        unset_wrap<Repeat> rep;

        /**
         * @brief Sizing. See ass::Sizing
         */
        unset_wrap<Sizing> sizing;

        /**
         * @brief Scale of the image by x and y axis. Default is `{ 1.0, 1.0 }`.
         */
        unset_wrap<glm::vec2> scale;

        /**
         * @brief DPI multiplier used to underscale or upperscale the icon.
         * @details
         * In example, you may use 64x64 png icons and set the dpiMargin to 2.0. They will be rendered as 32px icons on
         * 100% scale (instead of 64px), and 64px on 200% scale, remaining crisp. On 300% scale, however, they will be
         * rendered as 96px images, thus becoming blurry, hence usage of SVG icons is recommended.
         */
        unset_wrap<float> dpiMargin;

        BackgroundImage() {}

        BackgroundImage(std::nullptr_t): url("") {

        }

        BackgroundImage(const unset_wrap<AString>& url, const unset_wrap<AColor>& overlayColor):
                url(url),
                overlayColor(overlayColor) {}
        BackgroundImage(const AString& url):
                url(url) {}
        BackgroundImage(const AColor& overlayColor):
                overlayColor(overlayColor) {}

        BackgroundImage(const unset_wrap<AString>& url,
                        const unset_wrap<AColor>& overlayColor,
                        const unset_wrap<Repeat>& rep,
                        const unset_wrap<Sizing>& sizing = {},
                        const unset_wrap<glm::vec2>& scale = {},
                        const unset_wrap<float>& dpiMargin = {}) : url(url),
                                                                   overlayColor(overlayColor),
                                                                   rep(rep),
                                                                   sizing(sizing),
                                                                   scale(scale),
                                                                   dpiMargin(dpiMargin) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<BackgroundImage>: IPropertyBase, IStringable {
        private:
            BackgroundImage mInfo;

        public:
            Property(const BackgroundImage& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
            void renderFor(AView* view, const ARenderContext& ctx) override;

            PropertySlot getPropertySlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }

            AString toString() const override;


            static void draw(const ARenderContext& ctx, AView* view, const _<IDrawable>& drawable, const ass::BackgroundImage& info);

        };

    }
}

AUI_ENUM_VALUES(ass::Sizing,
                ass::Sizing::NONE,
                ass::Sizing::FIT,
                ass::Sizing::CENTER,
                ass::Sizing::FIT_PADDING,
                ass::Sizing::COVER,
                ass::Sizing::CONTAIN,
                ass::Sizing::SPLIT_2X2,
                ass::Sizing::CROPPED
)
