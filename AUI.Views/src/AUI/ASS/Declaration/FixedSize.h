//
// Created by alex2 on 03.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct FixedSize {
        AMetric width;
        AMetric height;

        FixedSize(const AMetric& width, const AMetric& height) : width(width), height(height) {}
        explicit FixedSize(const AMetric& side) : width(side), height(side) {}
    };


    namespace decl {
        template<>
        struct Declaration<FixedSize>: IDeclarationBase {
        private:
            FixedSize mInfo;

        public:
            Declaration(const FixedSize& info) : mInfo(info) {
            }

            void applyFor(AView* view) override;
        };
    }
}