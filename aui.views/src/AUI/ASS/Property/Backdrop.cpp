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
// Created by alex2 on 29.12.2020.
//

#include "Backdrop.h"
#include "AUI/Common/AMap.h"
#include "AUI/Render/IRenderer.h"
#include "IProperty.h"

void ass::prop::Property<ass::Backdrop>::renderFor(AView* view, const ARenderContext& ctx) {
    ctx.render.backdrops(view->getPosition(), view->getSize(), mInfo.effects);
}

bool ass::prop::Property<ass::Backdrop>::isNone() { return mInfo.effects.empty(); }

ass::prop::PropertySlot ass::prop::Property<ass::Backdrop>::getPropertySlot() const {
    return ass::prop::PropertySlot::BACKDROP;
}
