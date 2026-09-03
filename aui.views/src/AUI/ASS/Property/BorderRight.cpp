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

#include <AUI/Render/RenderHints.h>
#include "BorderRight.h"

void ass::legacy::Property<ass::BorderRight>::renderFor(AView* view, const ARenderContext& ctx) {
    RenderHints::PushColor x(ctx.render);
    int w = mInfo.width;

    ctx.render.rectangle(ASolidBrush{mInfo.color},
                         {view->getWidth() - w, 0},
                         {w, view->getHeight()});

}

bool ass::legacy::Property<ass::BorderRight>::isNone() {
    return mInfo.width <= 0.001f;
}

ass::legacy::PropertySlot ass::legacy::Property<ass::BorderRight>::getPropertySlot() const {
    return ass::legacy::PropertySlot::BORDER;
}
namespace ass {
Modifier operator|(Modifier thiz, BorderRight value) {
    return thiz.renderBehind([value = std::move(value)](ass::Modifier::RenderCtx ctx) {
        RenderHints::PushColor x(ctx.render);
        int w = value.width;
        ctx.render.rectangle(ASolidBrush{value.color},
                             {int(ctx.size.x) - w, 0},
                             {w, ctx.size.y});
    });
}
}
