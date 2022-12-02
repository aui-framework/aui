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
// Created by Alex2772 on 12/5/2021.
//


#include "UIMatcher.h"
#include <AUI/Platform/AWindow.h>
#include <AUI/Platform/AWindowManager.h>

ASet<_<AView>> UIMatcher::toSet() const {
    ASet<_<AView>> result;

    for (auto& window : ABaseWindow::getWindowManager().getWindows()) {
        processContainer(result, window);
    }
    return result;
}

UIMatcher*& UIMatcher::currentImpl() {
    thread_local UIMatcher* matcher;
    return matcher;
}

void UIMatcher::processContainer(ASet<_<AView>>& destination, const _<AViewContainer>& container) const {
    for (auto& view : container) {
        if (mIncludeInvisibleViews || view->getVisibility() == Visibility::VISIBLE) {
            if (mMatcher->matches(view)) {
                destination << view;
            }
            if (auto currentContainer = _cast<AViewContainer>(view)) {
                processContainer(destination, currentContainer);
            }
        }
    }
}
