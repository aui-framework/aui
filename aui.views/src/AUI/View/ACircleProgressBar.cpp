/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by Alex2772 on 1/8/2022.
//

#include "ACircleProgressBar.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/Render/ABrush.h"
#include "AUI/Render/RenderHints.h"
#include "AUI/Util/AAngleRadians.h"

ACircleProgressBar::ACircleProgressBar() : mInner(_new<Inner>()) {
    setLayout(std::make_unique<AStackedLayout>());
    addView(mInner);
}

ACircleProgressBar::Inner::~Inner() {

}

ACircleProgressBar::~ACircleProgressBar() {

}

void ACircleProgressBar::render(ARenderContext context) {
    AView::render(context); // NOLINT(*-parent-virtual-call)
    RenderHints::PushMask mask(context.render, [&] {
        context.render.squareSector(ASolidBrush{}, {0, 0}, getSize(), 0_deg, AAngleRadians(glm::radians(mValue * 360.f)));

    });
    AViewContainerBase::renderChildren(context);
}


_<AView> declarative::CircleProgressBar::operator()() {
    auto view = _new<ACircleProgressBar>();
    progress.bindTo(view->value().assignment());
    return view;
}
