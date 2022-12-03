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
// Created by Alex2772 on 8/31/2022.
//

#include "EffectShimmer.h"
#include <AUI/Render/Render.h>

void EffectShimmer::draw(AView* view) {
    using namespace std::chrono;

    const float t = (duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count() % 1'000) / 1'000.f;

    const auto transparent = AColor(glm::vec4(glm::vec3(mColor), 0.f));

    Render::rect(ALinearGradientBrush {
        transparent,
        mColor,
        transparent,
        mColor
    }, {(t * 4 - 2) * view->getSize().x, 0}, view->getSize());


    Render::rect(ALinearGradientBrush {
        mColor,
        transparent,
        mColor,
        transparent,
    }, {(t * 4 - 1) * view->getSize().x, 0}, view->getSize());

    view->redraw();
}
