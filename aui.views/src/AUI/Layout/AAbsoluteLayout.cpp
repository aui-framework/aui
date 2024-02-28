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

#include "AAbsoluteLayout.h"

void AAbsoluteLayout::onResize(int x, int y, int width, int height) {
    for (const auto& i : mViews) {
        i.view->setGeometry(i.pivotX.getValuePx(),
                            i.pivotY.getValuePx(),
                            i.sizeX ? i.sizeX->getValuePx() : i.view->getMinimumWidth(),
                            i.sizeY ? i.sizeY->getValuePx() : i.view->getMinimumHeight());
    }
}

int AAbsoluteLayout::getMinimumWidth() {
    int v = 0;
    for (const auto& i : mViews) {
        int x = i.pivotX.getValuePx();
        if (i.sizeX) {
            x += i.sizeX->getValuePx();
        }
        v = glm::max(v, x);
    }
    return v;
}

int AAbsoluteLayout::getMinimumHeight() {
    int v = 0;
    for (const auto& i : mViews) {
        int x = i.pivotY.getValuePx();
        if (i.sizeY) {
            x += i.sizeY->getValuePx();
        }
        v = glm::max(v, x);
    }
    return v;
}

void AAbsoluteLayout::add(aui::detail::AbsoluteLayoutCell cell) {
    mViews << std::move(cell);
}

void AAbsoluteLayout::addView(const _<AView>& view, AOptional<size_t> index) {
    addViewBasicImpl({
        .view = view
    }, index);
}
