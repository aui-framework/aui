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

//
// Created by Alex2772 on 1/8/2022.
//

#include "ACircleProgressBar.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/Render/ABrush.h"
#include "AUI/Render/RenderHints.h"

ACircleProgressBar::ACircleProgressBar() : mInner(_new<Inner>()) {
    setLayout(_new<AStackedLayout>());
    addView(mInner);
}

ACircleProgressBar::Inner::~Inner() {

}

ACircleProgressBar::~ACircleProgressBar() {

}

void ACircleProgressBar::render() {
    AView::render();
    RenderHints::PushMask mask([&] {
        ARender::squareSector(ASolidBrush{}, {0, 0}, getSize(), 0_deg, 60_deg);
    });
    AViewContainer::renderChildren();
}

