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
// Created by alex2 on 03.01.2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {
    /**
     * @brief Controls the expanding of AView.
     * @ingroup ass
     */
    struct Expanding {
        unset_wrap<unsigned> expandingX;
        unset_wrap<unsigned> expandingY;

        Expanding() : expandingX(2), expandingY(2) {}
        Expanding(const unset_wrap<unsigned>& expandingX, const unset_wrap<unsigned>& expandingY) : expandingX(expandingX), expandingY(expandingY) {}
        Expanding(const unset_wrap<unsigned>& expanding) : expandingX(expanding), expandingY(expanding) {}
    };


    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<Expanding>: IDeclarationBase {
        private:
            Expanding mInfo;

        public:
            Declaration(const Expanding& info) : mInfo(info) {
            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}