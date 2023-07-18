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
// Created by alex2 on 08.01.2021.
//

#pragma once

#include <AUI/ASS/Property/IProperty.h>
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


    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<BorderBottom> : IPropertyBase {
        private:
            BorderBottom mInfo;

        public:
            Property(const BorderBottom& info) : mInfo(info) {

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