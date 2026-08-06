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
// Created by alex2 on 28.11.2020.
//

#include "APlaceholderAnimator.h"
#include <AUI/View/AView.h>
#include <AUI/Util/AMetric.h>
#include <AUI/Render/ACanvas.hpp>

APlaceholderAnimator::APlaceholderAnimator() {

}

void APlaceholderAnimator::doPostRender(AView* view, float theta, ACanvas& render) {
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
}
