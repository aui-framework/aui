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

#include "BoxShadowInnerV2.h"
#include <AUI/Render/IRenderer.h>

namespace ass {

Modifier operator|(Modifier thiz, BoxShadowInnerV2 value) {
    return thiz.renderBehind([value = std::move(value)](Modifier::RenderCtx ctx) {
        auto size = glm::vec2(ctx.size);
        float borderRadius = std::visit(aui::lambda_overloaded{
            [](const AShapeRoundedRectangle& r) { return r.radius.getValuePx(); },
            [](const AShapeRectangle&) { return 0.f; },
        }, value.shape);
        ctx.render.boxShadowInner(
            {0, 0},
            size,
            value.blurRadius,
            value.spreadRadius,
            borderRadius,
            value.color,
            {value.offsetX.getValuePx(), value.offsetY.getValuePx()});
    });
}

}   // namespace ass
