//
// Created by alex2 on 01.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct MinSize {
        AMetric width;
        AMetric height;

        MinSize(const AMetric& width, const AMetric& height) : width(width), height(height) {}
    };

    namespace decl {
        template<>
        struct Declaration<MinSize>: IDeclarationBase {
        private:
            MinSize mInfo;

        public:
            Declaration(const MinSize& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;
        };
    }
}