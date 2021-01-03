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
        AString url;
        AColor color = 0xffffff_rgb;
        Repeat repeat = REPEAT_NONE;
        Sizing sizing = Sizing::NONE;
    };

    namespace decl {
        template<>
        struct Declaration<BackgroundUrl>: IDeclarationBase {
        private:
            BackgroundUrl mInfo;

        public:
            Declaration(const BackgroundUrl& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;

            DeclarationSlot getDeclarationSlot() const override;
        };

    }
}