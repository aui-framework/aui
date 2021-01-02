//
// Created by alex2 on 02.01.2021.
//

#pragma once

#include "IDeclaration.h"

namespace ass {
    struct TextColor {
        AColor color;
    };


    namespace decl {
        template<>
        struct Declaration<TextColor>: IDeclarationBase {
        private:
            TextColor mInfo;

        public:
            Declaration(const TextColor& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}