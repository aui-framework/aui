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
// Created by alex2 on 22.09.2020.
//

#include "ASizeAnimator.h"
#include <AUI/View/AView.h>

ASizeAnimator::ASizeAnimator(const glm::ivec2& beginSize, const glm::ivec2& endSize):
    mBeginSize(beginSize), mEndSize(endSize) {

}

void ASizeAnimator::doAnimation(AView* view, float theta) {
    if (mEndSize.x == 0 && mEndSize.y == 0) {
        mEndSize = view->getSize();
    }
    auto currentSize = glm::mix(mBeginSize, mEndSize, theta);
    view->setSizeForced(currentSize);
    view->redraw();
}

