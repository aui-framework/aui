/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
 
 * Original code located at https://github.com/Alex2772/aui
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
        FIT,
        FIT_PADDING,
    };

    struct BackgroundImage {
        unset_wrap<AString> url;
        unset_wrap<AColor> overlayColor;
        unset_wrap<Repeat> rep;
        unset_wrap<Sizing> sizing;

        BackgroundImage() {}

        BackgroundImage(const AString& url) : url(url) {}
        BackgroundImage(const AColor& overlayColor) : overlayColor(overlayColor) {}

        BackgroundImage(const AString& url,
                        const AColor& overlayColor,
                        Repeat repeat,
                        Sizing sizing):
            url(url),
            overlayColor(overlayColor),
            rep(repeat),
            sizing(sizing) {}
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
        };

    }
}