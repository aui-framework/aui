//
// Created by alex2 on 01.01.2021.
//

#pragma once


#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    struct Border {
        enum BorderType {
            SOLID,
        };

        AMetric width;
        BorderType type;
        AColor color;

        Border(const AMetric& width,
               BorderType type,
               const AColor& color):
            width(width),
            type(type),
            color(color) {

        }
        Border(const AMetric& width,
               const AColor& color):
            width(width),
            type(SOLID),
            color(color) {

        }
        Border(std::nullptr_t) {}

    };

    namespace decl {
        template<>
        struct Declaration<Border>: IDeclarationBase {
        private:
            Border mInfo;

        public:
            Declaration(const Border& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;

            bool isNone() override;

            DeclarationSlot getDeclarationSlot() const override;
        };
    }
}