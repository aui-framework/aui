// AUI Framework - Declarative UI toolkit for modern C++17
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
// Created by alex2 on 07.01.2021.
//


#pragma once


#include "IDeclaration.h"

namespace ass {

    /**
     * @brief Controls the rendering scale transform of AView.
     * @ingroup ass
     */
    struct TransformScale {
        float scaleX;
        float scaleY;

        TransformScale(float scaleX, float scaleY) : scaleX(scaleX), scaleY(scaleY) {}
        explicit TransformScale(float scale) : scaleX(scale), scaleY(scale) {}
    };


    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<TransformScale>: IDeclarationBase {
        private:
            TransformScale mInfo;

        public:
            Declaration(const TransformScale& info) : mInfo(info) {}
            void renderFor(AView* view) override;

            DeclarationSlot getDeclarationSlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}