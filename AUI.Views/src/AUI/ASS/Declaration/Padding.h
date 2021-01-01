//
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct Padding {
        ABoxFields padding;

        Padding(AMetric all) {
            padding.left = all;
            padding.top = all;
            padding.right = all;
            padding.bottom = all;
        }
        Padding(AMetric vertical, AMetric horizontal) {
            padding.left = horizontal;
            padding.top = vertical;
            padding.right = horizontal;
            padding.bottom = vertical;
        }
        Padding(AMetric top, AMetric horizontal, AMetric bottom) {
            padding.left = horizontal;
            padding.top = top;
            padding.right = horizontal;
            padding.bottom = bottom;
        }
        Padding(AMetric top, AMetric right, AMetric bottom, AMetric left) {
            padding.left = left;
            padding.top = top;
            padding.right = right;
            padding.bottom = bottom;
        }
    };

    namespace decl {
        template<>
        struct Declaration<Padding>: IDeclarationBase {
        private:
            Padding mInfo;

        public:
            Declaration(const Padding& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}