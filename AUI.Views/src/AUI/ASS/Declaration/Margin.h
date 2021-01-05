//
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct Margin {
        ABoxFields margin;

        Margin(AMetric all) {
            margin.left = all;
            margin.top = all;
            margin.right = all;
            margin.bottom = all;
        }
        Margin(AMetric vertical, AMetric horizontal) {
            margin.left = horizontal;
            margin.top = vertical;
            margin.right = horizontal;
            margin.bottom = vertical;
        }
        Margin(AMetric top, AMetric horizontal, AMetric bottom) {
            margin.left = horizontal;
            margin.top = top;
            margin.right = horizontal;
            margin.bottom = bottom;
        }
        Margin(AMetric top, AMetric right, AMetric bottom, AMetric left) {
            margin.left = left;
            margin.top = top;
            margin.right = right;
            margin.bottom = bottom;
        }
    };

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<Margin>: IDeclarationBase {
        private:
            Margin mInfo;

        public:
            Declaration(const Margin& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}