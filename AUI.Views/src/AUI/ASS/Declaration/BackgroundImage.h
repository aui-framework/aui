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