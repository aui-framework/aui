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

#include "DemoGraphView.h"
#include <AUI/ASS/ASS.h>

constexpr auto POINT_COUNT = 100.f;

DemoGraphView::DemoGraphView()
{
    setFixedSize({ 5_dp * POINT_COUNT, 100_dp }); // set fixed size


    mPoints.reserve(POINT_COUNT);
    for (std::size_t i = 0; i < POINT_COUNT; ++i) {
        // map a sinusoid to view
        mPoints << glm::vec2{ 5_dp * float(i), 100_dp * ((glm::sin(i / 10.f) + 1.f) / 2.f) };
    }
}

void DemoGraphView::render(ClipOptimizationContext context) {
    AView::render(context);

    ARender::lines(ASolidBrush{0xff0000_rgb }, mPoints, ABorderStyle::Dashed{}, 4_dp);

}
