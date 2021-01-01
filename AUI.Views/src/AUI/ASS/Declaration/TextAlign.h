//
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    namespace decl {
        template<>
        struct Declaration<TextAlign>: IDeclarationBase {
        private:
            TextAlign mInfo;

        public:
            Declaration(const TextAlign& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}