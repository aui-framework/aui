/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//
// Created by alex2 on 01.01.2021.
//

#include "BoxShadowInner.h"
#include "AUI/Render/ARender.h"


void ass::prop::Property<ass::BoxShadowInner>::renderFor(AView* view, const ARenderContext& ctx) {
    ctx.render.boxShadowInner({0, 0},
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