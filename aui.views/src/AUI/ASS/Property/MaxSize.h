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
#include "AUI/Common/IStringable.h"
#include "IProperty.h"

namespace ass {


    /**
     * @brief Controls the max size of AView.
     * @ingroup ass
     */
    struct MaxSize {
        unset_wrap<AMetric> width;
        unset_wrap<AMetric> height;

        MaxSize(const unset_wrap <AMetric>& width, const unset_wrap <AMetric>& height) : width(width), height(height) {}
        explicit MaxSize(const AMetric& side) : width(side), height(side) {}
    };

    namespace prop {
        template<>
        struct API_AUI_VIEWS Property<MaxSize>: IPropertyBase, IStringable {
        private:
            MaxSize mInfo;

        public:
            Property(const MaxSize& info) : mInfo(info) {

            }

            void applyFor(AView* view) override;

            [[nodiscard]]
            const auto& value() const noexcept {
                return mInfo;
            }

            AString toString() const override {
                return "MaxSize({}, {})"_format(mInfo.width, mInfo.height);
            }
        };
    }
}