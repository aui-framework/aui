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

#include "BackgroundSolidV2.h"
#include <AUI/Render/IRenderer.h>
#include <AUI/Render/ABrush.h>

namespace ass {

Modifier operator|(Modifier thiz, BackgroundSolidV2 value) {
    return thiz.renderBehind([value = std::move(value)](Modifier::RenderCtx ctx) {
        ASolidBrush brush{value.color};
        auto size = glm::vec2(ctx.size);
        std::visit(aui::lambda_overloaded{
            [&](const AShapeRoundedRectangle& rounded) {
                ctx.render.roundedRectangle(brush, {0, 0}, size, rounded.radius.getValuePx());
            },
            [&](const AShapeRectangle&) {
                ctx.render.rectangle(brush, {0, 0}, size);
            },
        }, value.shape);
    });
}

} // namespace ass
