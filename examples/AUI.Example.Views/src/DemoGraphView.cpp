/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
