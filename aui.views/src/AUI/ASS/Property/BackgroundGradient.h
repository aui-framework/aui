// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
// Created by alex2 on 29.12.2020.
//

#pragma once

#include <AUI/Util/ALayoutDirection.h>
#include "IProperty.h"

namespace ass {
    /**
     * @brief Represents gradient background.
     * @ingroup ass
     */
    struct BackgroundGradient {
        AOptional<ALinearGradientBrush> gradient;

        BackgroundGradient(std::nullptr_t) noexcept {};
        BackgroundGradient(ALinearGradientBrush brush) noexcept: gradient(std::move(brush)) {}

        BackgroundGradient(AColor begin, AColor end, AAngleRadians angle) noexcept: gradient(ALinearGradientBrush{
                .colors = {
                        {0.f, begin},
                        {1.f, end},
                },
                .rotation = angle
        }) {}
        BackgroundGradient(AColor begin, AColor end, ALayoutDirection direction) noexcept: BackgroundGradient(begin, end, direction == ALayoutDirection::VERTICAL ? 180_deg : 90_deg) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<BackgroundGradient>: IPropertyBase {
        private:
            BackgroundGradient mInfo;

        public:
            Property(const BackgroundGradient& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;

            PropertySlot getPropertySlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }

        };

    }
}