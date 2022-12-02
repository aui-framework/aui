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
// Created by alex2 on 28.11.2020.
//

#include "APlaceholderAnimator.h"
#include <AUI/View/AView.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Render/Render.h>

APlaceholderAnimator::APlaceholderAnimator() {

}

void APlaceholderAnimator::doPostRender(AView* view, float theta) {
    float width;
    if (auto cs = dynamic_cast<ICustomWidth*>(view)) {
        width = cs->getCustomWidthForPlaceholderAnimator();
    } else {
        width = view->getWidth();
    }
    width = glm::max(width, 80.f);
    setDuration(width / float(200_dp));

    auto BRIGHT_COLOR = 0x40ffffff_argb;

    const float WIDTH = 200;
    float posX = theta * (view->getWidth() + WIDTH * 2.f) - WIDTH;

    Render::rect(
            ALinearGradientBrush{
                    0x00ffffff_argb, BRIGHT_COLOR,
                    0x00ffffff_argb, BRIGHT_COLOR
            },
            {posX, 0},
            {WIDTH / 2, view->getHeight()});

    Render::rect(
            ALinearGradientBrush{
                    BRIGHT_COLOR, 0x00ffffff_argb,
                    BRIGHT_COLOR, 0x00ffffff_argb
            },
            {posX + WIDTH / 2, 0},
            {WIDTH / 2, view->getHeight()});
}
