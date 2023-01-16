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
// Created by alex2 on 29.12.2020.
//

#pragma once

#include <AUI/Util/ALayoutDirection.h>
#include "IDeclaration.h"
#include <AUI/Enum/ImageRendering.h>

namespace ass {

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<ImageRendering>: IDeclarationBase {
        private:
            ImageRendering mInfo;

        public:
            Declaration(const ImageRendering& info) : mInfo(info) {

            }

            void renderFor(AView* view) override;
            DeclarationSlot getDeclarationSlot() const override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };

    }
}