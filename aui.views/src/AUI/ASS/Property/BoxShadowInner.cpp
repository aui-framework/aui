// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include "BoxShadowInner.h"
#include "AUI/Render/ARender.h"


void ass::prop::Property<ass::BoxShadowInner>::renderFor(AView* view) {
    ARender::boxShadowInner({0, 0},
                             glm::vec2(view->getSize()),
                             mInfo.blurRadius,
                             mInfo.spreadRadius,
                             view->getBorderRadius(),
                             mInfo.color,
                             {mInfo.offsetX.getValuePx(), mInfo.offsetY.getValuePx()});
}

ass::prop::PropertySlot ass::prop::Property<ass::BoxShadowInner>::getPropertySlot() const {
    return ass::prop::PropertySlot::SHADOW_INNER;
}

bool ass::prop::Property<ass::BoxShadowInner>::isNone() {
    return mInfo.color.isFullyTransparent();
}