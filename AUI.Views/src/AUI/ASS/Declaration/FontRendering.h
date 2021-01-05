//
// Created by alex2 on 29.12.2020.
//

#pragma once

#include "IDeclaration.h"

namespace ass {
    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<FontRendering>: IDeclarationBase {
        private:
            FontRendering mInfo;

        public:
            Declaration(const FontRendering& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };

    }
}