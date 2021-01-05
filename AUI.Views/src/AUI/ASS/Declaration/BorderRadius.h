//
// Created by alex2 on 01.01.2021.
//

#pragma once


#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct BorderRadius {
        AMetric radius;
    };

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<BorderRadius>: IDeclarationBase {
        private:
            BorderRadius mInfo;

        public:
            Declaration(const BorderRadius& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}