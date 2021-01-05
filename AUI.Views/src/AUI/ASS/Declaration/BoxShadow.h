//
// Created by alex2 on 01.01.2021.
//

#pragma once


#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct BoxShadow {
        AMetric offsetX;
        AMetric offsetY;
        AMetric blurRadius;
        AMetric spreadRadius;
        AColor color;

        BoxShadow(AMetric offsetX,
                  AMetric offsetY,
                  AMetric blurRadius,
                  AMetric spreadRadius,
                  const AColor& color):
            offsetX(offsetX),
            offsetY(offsetY),
            blurRadius(blurRadius),
            spreadRadius(spreadRadius),
            color(color) {}

        BoxShadow(AMetric offsetX,
                  AMetric offsetY,
                  AMetric blurRadius,
                  const AColor& color):
            offsetX(offsetX),
            offsetY(offsetY),
            blurRadius(blurRadius),
            color(color) {}

        BoxShadow(std::nullptr_t): color(0.f) {}
    };

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<BoxShadow>: IDeclarationBase {
        private:
            BoxShadow mInfo;

        public:
            Declaration(const BoxShadow& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;

            bool isNone() override;

            DeclarationSlot getDeclarationSlot() const override;
        };
    }
}