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
// Created by alex2 on 18.09.2020.
//

#include "AFocusAnimator.h"
#include <AUI/Render/Render.h>
#include <AUI/View/AView.h>
#include <AUI/Render/RenderHints.h>
#include <AUI/Platform/AWindow.h>

AFocusAnimator::AFocusAnimator() {
}

void AFocusAnimator::doAnimation(AView* view, float theta) {
    const float SIZE = 4.f;
    if (theta < 0.99999f) {
        float t = glm::pow(1.f - theta, 4.f);
        Render::rectBorder(
                ASolidBrush{{0, 0, 0, t}},
                {-t * SIZE, -t * SIZE},
                {t * SIZE * 2 + view->getWidth(), t * SIZE * 2 + view->getHeight()},
                2.f);
    }
}
