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
// Created by alex2 on 01.01.2021.
//

#include "Border.h"
#include <AUI/Render/IRenderer.h>
#include <AUI/Render/RenderHints.h>


void ass::prop::Property<ass::Border>::renderFor(AView* view, const ARenderContext& ctx) {
    /*
    if (view->getBorderRadius() < 0.1f) {
        ctx.render.drawRectBorder(ASolidBrush{mInfo.color},
                               {0, 0},
                               view->getSize(),
                               mInfo.width);
    } else {
        */
    ctx.render.roundedRectangleBorder(ASolidBrush{mInfo.color},
                                      {0, 0},
                                      view->getSize(),
                                      view->getBorderRadius(),
                                      mInfo.width);
    //}
}

bool ass::prop::Property<ass::Border>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::prop::PropertySlot ass::prop::Property<ass::Border>::getPropertySlot() const {
    return ass::prop::PropertySlot::BORDER;
}