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

    struct BackgroundUrl {
        unset_wrap<AString> url;
        unset_wrap<AColor> overlayColor;
        unset_wrap<Repeat> repeat;
        unset_wrap<Sizing> sizing;
    };

    namespace decl {
        template<>
        struct Declaration<BackgroundUrl>: IDeclarationBase {
        private:
            BackgroundUrl mInfo;

        public:
            Declaration(const BackgroundUrl& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
            void renderFor(AView* view) override;

            DeclarationSlot getDeclarationSlot() const override;
        };

    }
}