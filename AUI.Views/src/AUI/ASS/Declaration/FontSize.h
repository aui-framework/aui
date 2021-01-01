//
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct FontSize {
        AMetric size;
    };

    namespace decl {
        template<>
        struct Declaration<FontSize>: IDeclarationBase {
        private:
            FontSize mInfo;

        public:
            Declaration(const FontSize& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}