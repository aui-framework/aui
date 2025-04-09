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

#include "TextShadow.h"
#include <AUI/View/AAbstractLabel.h>
#include <AUI/Render/RenderHints.h>

void ass::prop::Property<ass::TextShadow>::renderFor(AView* view, const ARenderContext& ctx) {
    if (auto label = dynamic_cast<AAbstractLabel*>(view)) {
        RenderHints::PushColor c(ctx.render);
        ctx.render.setColor(mInfo.shadowColor);

        {
            RenderHints::PushMatrix m(ctx.render);
            ctx.render.translate({mInfo.offsetX.getValuePx(), mInfo.offsetY.getValuePx()});
            label->doRenderText(ctx.render);
        }
    }
}
ass::prop::PropertySlot ass::prop::Property<ass::TextShadow>::getPropertySlot() const {
    return ass::prop::PropertySlot::TEXT_SHADOW;
}