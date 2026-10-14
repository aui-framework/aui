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
// Created by alex2 on 08.01.2021.
//

#include <AUI/Render/RenderHints.h>
#include "BorderBottom.h"

void ass::legacy::Property<ass::BorderBottom>::renderFor(AView* view, const ARenderContext& ctx) {
    RenderHints::PushColor x(ctx.render);
    int w = mInfo.width;
    ctx.render.rectangle(ASolidBrush{mInfo.color},
                         {0, view->getHeight() - w},
                         {view->getWidth(), w});
}

bool ass::legacy::Property<ass::BorderBottom>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::legacy::PropertySlot ass::legacy::Property<ass::BorderBottom>::getPropertySlot() const {
    return ass::legacy::PropertySlot::BORDER;
}
namespace ass {
Modifier operator|(Modifier thiz, BorderBottom value) {
    return thiz.renderBehind([value = std::move(value)](ass::Modifier::RenderCtx ctx) {
        RenderHints::PushColor x(ctx.render);
        int w = value.width;
        ctx.render.rectangle(ASolidBrush{value.color},
                             {0, int(ctx.size.y) - w},
                             {ctx.size.x, w});
    });
}
}   // namespace ass
