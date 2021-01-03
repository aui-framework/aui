//
// Created by alex2 on 29.12.2020.
//

#pragma once

#include "IDeclaration.h"

namespace ass {
    struct BackgroundSolid {
        AColor color;

        BackgroundSolid(const AColor& color) : color(color) {}
        BackgroundSolid(std::nullptr_t) : color(0.f) {}
    };

    namespace decl {
        template<>
        struct Declaration<BackgroundSolid>: IDeclarationBase {
        private:
            BackgroundSolid mInfo;

        public:
            Declaration(const BackgroundSolid& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;

            bool isNone() override;

            DeclarationSlot getDeclarationSlot() const override;
        };

    }
}