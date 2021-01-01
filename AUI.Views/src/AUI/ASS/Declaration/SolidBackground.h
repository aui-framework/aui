//
// Created by alex2 on 29.12.2020.
//

#pragma once

#include "IDeclaration.h"

namespace ass {
    struct SolidBackground {
        AColor color;
    };

    namespace decl {
        template<>
        struct Declaration<SolidBackground>: IDeclarationBase {
        private:
            SolidBackground mInfo;

        public:
            Declaration(const SolidBackground& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            void renderFor(AView* view) override;

            DeclarationSlot getDeclarationSlot() const override;
        };

    }
}