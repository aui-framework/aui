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

#pragma once


#include <AUI/Platform/ACursor.h>
#include <AUI/Util/AMetric.h>
#include "IDeclaration.h"

namespace ass {

    /**
     * @brief Controls the @ref ALayout::setSpacing "layout spacing" expanding of AViewContainer's layout.
     * @ingroup ass
     */
    struct LayoutSpacing {
        AMetric spacing;
    };

    namespace decl {
        template<>
        struct API_AUI_VIEWS Declaration<LayoutSpacing>: IDeclarationBase {
        private:
            LayoutSpacing mInfo;

        public:
            Declaration(const LayoutSpacing& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }
        };
    }
}