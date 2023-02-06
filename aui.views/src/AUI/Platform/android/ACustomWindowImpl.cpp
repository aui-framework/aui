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

#include "AUI/Platform/ACustomWindow.h"
#include "AUI/Platform/ADesktop.h"
#include <cstring>
#include <AUI/View/AButton.h>

const int AUI_TITLE_HEIGHT = 30;


void ACustomWindow::handleXConfigureNotify() {

}

ACustomWindow::ACustomWindow(const AString &name, int width, int height) {

}

void ACustomWindow::onMousePressed(const APointerPressedEvent& event) {
    ABaseWindow::onMousePressed(event);
}

void ACustomWindow::onMouseReleased(const APointerReleasedEvent& event) {
    ABaseWindow::onMouseReleased(event);
}


bool ACustomWindow::isCaptionAt(const glm::ivec2& pos) {
    if (pos.y <= AUI_TITLE_HEIGHT) {
        if (auto v = getViewAtRecursive(pos)) {
            if (!(_cast<AButton>(v)) &&
                !v->getAssNames().contains(".override-title-dragging")) {
                return true;
            }
        }
    }
    return false;
}