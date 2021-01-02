//
// Created by alex2 on 29.12.2020.
//

#pragma once

#include <AUI/Util/LayoutDirection.h>
#include "IDeclaration.h"

namespace ass {
    struct GradientBackground {
        AColor topLeftColor;
        AColor bottomRightColor;
        LayoutDirection direction;
    };

    namespace decl {
        template<>
        struct Declaration<GradientBackground>: IDeclarationBase {
        private:
            GradientBackground mInfo;

        public:
            Declaration(const GradientBackground& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;

            DeclarationSlot getDeclarationSlot() const override;
        };

    }
}