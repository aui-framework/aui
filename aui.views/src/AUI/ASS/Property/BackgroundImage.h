// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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
// Created by alex2 on 3.12.2020.
//

#pragma once

#include <AUI/Render/Render.h>
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
        unset_wrap<AString> url;
        unset_wrap<AColor> overlayColor;
        unset_wrap<Repeat> rep;
        unset_wrap<Sizing> sizing;
        unset_wrap<glm::vec2> scale;
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
            void renderFor(AView* view) override;

            PropertySlot getPropertySlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }

            AString toString() const override;

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
