//
// Created by alex2 on 08.01.2021.
//

#pragma once

#include <AUI/ASS/Declaration/IDeclaration.h>
#include <AUI/Util/AMetric.h>

namespace ass {

    /**
     * @brief Represents bottom border.
     * @ingroup ass
     */
    struct BorderBottom {
        AMetric width;
        AColor color;

        BorderBottom(std::nullptr_t): width(0) {}
        BorderBottom(const AMetric &width, const AColor &color) : width(width), color(color) {}
    };


    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<BorderBottom> : IDeclarationBase {
        private:
            BorderBottom mInfo;

        public:
            Declaration(const BorderBottom& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;

            bool isNone() override;

            DeclarationSlot getDeclarationSlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}