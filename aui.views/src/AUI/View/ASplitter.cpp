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

#include "ASplitter.h"
#include <AUI/Util/UIBuildingHelpers.h>

ASplitter::ASplitter() {
}

void ASplitter::onPointerPressed(const APointerPressedEvent& event) {
    AViewContainer::onPointerPressed(event);
    if (getViewAt(event.position) == nullptr) {
        mHelper.beginDrag(event.position);
    }
}


void ASplitter::setSize(glm::ivec2 size) {
    AViewContainer::setSize(size);
}

void ASplitter::onMouseMove(glm::ivec2 pos) {
    AViewContainer::onMouseMove(pos);
    if (mHelper.mouseDrag(pos)) {
        updateLayout();
        redraw();
    }
}

void ASplitter::onPointerReleased(const APointerReleasedEvent& event) {
    AViewContainer::onPointerReleased(event);
    mHelper.endDrag();
}
