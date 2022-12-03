// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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