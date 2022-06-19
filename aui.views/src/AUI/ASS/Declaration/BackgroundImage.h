/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

//
// Created by alex2 on 3.12.2020.
//

#pragma once

#include <AUI/Render/Render.h>
#include "IDeclaration.h"

namespace ass {
    enum class Sizing {
        NONE,

        /**
         * Resize image to view's area without keeping aspect ratio.
         */
        FIT,

        /**
         * Center the image
         */
        CENTER,

        /**
         * Resize image to view's content area without keeping aspect ratio.
         */
        FIT_PADDING,

        /**
         * Resize image to view's area keeping aspect ratio and cutting of excess parts. Matches CSS
         * background-size: cover
         */
        COVER,

        /**
         * Resize image to view's area keeping aspect ratio and keeping space not covered by the image. Matches CSS
         * background-size: contain
         */
        CONTAIN,

        /**
         * Texture divided to 4 parts cutting center.
         */
        SPLIT_2X2,

        /**
         * Texture is cropped by BackgroundCropping rule.
         */
        CROPPED,
    };

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

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<BackgroundImage>: IDeclarationBase {
        private:
            BackgroundImage mInfo;

        public:
            Declaration(const BackgroundImage& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
            void renderFor(AView* view) override;

            DeclarationSlot getDeclarationSlot() const override;

            const BackgroundImage& value() const { return mInfo; }
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
