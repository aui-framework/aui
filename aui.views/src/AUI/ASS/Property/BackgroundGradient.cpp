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
// Created by alex2 on 29.12.2020.
//

#include <AUI/Render/RenderHints.h>
#include "BackgroundGradient.h"
#include "IProperty.h"

void ass::legacy::Property<ass::BackgroundGradient>::renderFor(AView* view, const ARenderContext& ctx) {
    if (!mInfo.gradient) {
        return;
    }
    RenderHints::PushColor x(ctx.render);

    if (view->getBorderRadius() > 0) {
        ctx.render.roundedRectangle(ABrush(*mInfo.gradient), { 0, 0 }, view->getSize(), view->getBorderRadius());
    } else {
        ctx.render.rectangle(ABrush(*mInfo.gradient), { 0, 0 }, view->getSize());
    }
    IPropertyBase::renderFor(view, ctx);
}

ass::legacy::PropertySlot ass::legacy::Property<ass::BackgroundGradient>::getPropertySlot() const {
    return ass::legacy::PropertySlot::BACKGROUND_SOLID;
}
namespace ass {
Modifier operator|(Modifier thiz, BackgroundGradient value) {
    return thiz.renderBehind([value = std::move(value)](ass::Modifier::RenderCtx ctx) {
        if (!value.gradient) {
            return;
        }
        RenderHints::PushColor x(ctx.render);

        ctx.render.rectangle(ABrush(*value.gradient), { 0, 0 }, glm::vec2(ctx.size));
    });
}
}   // namespace ass
