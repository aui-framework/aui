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
// Created by alex2 on 29.12.2020.
//

#pragma once

#include "IProperty.h"

namespace ass {

    /**
     * @brief Represents solid (single color) background.
     * @ingroup ass
     */
    struct BackgroundSolid {
        AColor color;

        BackgroundSolid(const AColor& color) : color(color) {}
        BackgroundSolid(std::nullptr_t) : color(0.f) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<BackgroundSolid>: IPropertyBase {
        private:
            BackgroundSolid mInfo;

        public:
            Property(const BackgroundSolid& info) : mInfo(info) {

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