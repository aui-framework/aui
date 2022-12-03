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

#pragma once

#include <AUI/api.h>
#include <AUI/View/AView.h>

namespace uitest {
    API_AUI_UITESTS void frame();

    namespace impl {
        template<int axisIndex>
        inline int axisAlignedDistanceBetween(const _<AView>& v1, const _<AView>& v2) {
            auto begin1 = v1->getPositionInWindow()[axisIndex];
            auto begin2 = v2->getPositionInWindow()[axisIndex];
            auto end1   = begin1 + v1->getSize()[axisIndex];
            auto end2   = begin2 + v2->getSize()[axisIndex];

            return glm::min(glm::clamp(begin2, begin1, end1) - begin2,
                            glm::clamp(end2, begin1, end1) - end2);
        }
    }

    inline int horizontalDistanceBetween(const _<AView>& v1, const _<AView>& v2) {
        return impl::axisAlignedDistanceBetween<0>(v1, v2);
    }
    inline int verticalDistanceBetween(const _<AView>& v1, const _<AView>& v2) {
        return impl::axisAlignedDistanceBetween<1>(v1, v2);
    }
}