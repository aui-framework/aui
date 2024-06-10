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

#include <initializer_list>
#include "IProperty.h"

namespace ass {

    /**
     * @brief Represents backdrop filter effect which applied to the pixels behind the view (i.e., blur, grayscale, etc.)
     * @ingroup ass
     * @details
     * Matches CSS property `backdrop-filter`.
     *
     * Because the effect is applied to everything behind the view, to see the effect the view's background needs to be
     * transparent or partially transparent.
     */
    struct Backdrop {
        struct GaussianBlur {
            int radius = 13;
        };

        using Any = std::variant<GaussianBlur>;
        AVector<Any> effects;

        Backdrop(std::initializer_list<Any> effects): effects(std::move(effects)) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<Backdrop>: IPropertyBase {
        private:
            Backdrop mInfo;

        public:
            Property(const Backdrop& info) : mInfo(info) {

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