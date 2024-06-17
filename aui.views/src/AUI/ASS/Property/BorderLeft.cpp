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
// Created by alex2 on 08.01.2021.
//

#include <AUI/Render/RenderHints.h>
#include "BorderLeft.h"

void ass::prop::Property<ass::BorderLeft>::renderFor(AView* view) {
    RenderHints::PushColor x;
    int w = mInfo.width;

    ARender::rect(ASolidBrush{mInfo.color},
                  {0, 0},
                  {w, view->getHeight()});

}

bool ass::prop::Property<ass::BorderLeft>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::prop::PropertySlot ass::prop::Property<ass::BorderLeft>::getPropertySlot() const {
    return ass::prop::PropertySlot::BORDER;
}