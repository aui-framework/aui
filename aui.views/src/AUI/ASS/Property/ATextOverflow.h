//
// Created by dervisdev on 1/20/2023.
//

#pragma once

#include <AUI/Platform/ACursor.h>
#include <AUI/Util/AMetric.h>
#include "IProperty.h"

namespace ass {
    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<ATextOverflow>: IPropertyBase {
        private:
            ATextOverflow mInfo;

        public:
            Property(const ATextOverflow& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}
