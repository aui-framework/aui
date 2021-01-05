//
// Created by alex2 on 29.12.2020.
//

#pragma once

#include <AUI/Util/LayoutDirection.h>
#include "IDeclaration.h"

namespace ass {
    struct BackgroundGradient {
        AColor topLeftColor;
        AColor bottomRightColor;
        LayoutDirection direction;
    };

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<BackgroundGradient>: IDeclarationBase {
        private:
            BackgroundGradient mInfo;

        public:
            Declaration(const BackgroundGradient& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;

            DeclarationSlot getDeclarationSlot() const override;
        };

    }
}