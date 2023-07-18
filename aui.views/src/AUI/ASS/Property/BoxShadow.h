// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 01.01.2021.
//

#pragma once


#include <AUI/Util/AMetric.h>
#include "IProperty.h"

namespace ass {

    /**
     * @brief Represents box shadow.
     * @ingroup ass.
     * @note Unlike CSS, box shadow is not affected by BorderRadius.
     */
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

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<BoxShadow>: IPropertyBase {
        private:
            BoxShadow mInfo;

        public:
            Property(const BoxShadow& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;

            bool isNone() override;

            PropertySlot getPropertySlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}