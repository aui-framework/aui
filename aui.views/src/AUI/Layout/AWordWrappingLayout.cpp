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
// Created by Alex2772 on 9/7/2021.
//

#include "AWordWrappingLayout.h"


void AWordWrappingLayout::addView(const _<AView>& view, AOptional<size_t> index) {
    ALinearLayout::addView(view, index);

    if (index) {
        if (mViewEntry.size() <= *index) {
            mViewEntry.resize(*index + 1);
        }
        mViewEntry[*index] = AViewEntry{ view };
    } else {
        mViewEntry << AViewEntry{ view };
    }
}

void AWordWrappingLayout::removeView(aui::no_escape<AView> view, size_t index) {
    ALinearLayout::removeView(view, index);

    mViewEntry.removeAt(index);
}



int AWordWrappingLayout::getMinimumWidth() {
    return 0;
}

int AWordWrappingLayout::getMinimumHeight() {
    int m = 0;
    for (auto& view : mViews) {
        m = (glm::max)(view->getPosition().y + view->getSize().y, m);
    }
    return m;
}

void AWordWrappingLayout::onResize(int x, int y, int width, int height) {
    AVector<_<AWordWrappingEngine::Entry>> entries;
    for (auto& v : mViewEntry) {
        entries << aui::ptr::fake(&v);
    }
    AWordWrappingEngine we;
    we.setEntries(std::move(entries));
    we.performLayout({x, y}, {width, height});
}