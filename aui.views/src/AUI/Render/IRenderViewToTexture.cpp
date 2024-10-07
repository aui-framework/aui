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

#include "IRenderViewToTexture.h"
#include <AUI/View/AView.h>
#include <AUI/Render/IRenderer.h>

void IRenderViewToTexture::enableForView(IRenderer& renderer, AView& view) {
    auto renderViewToTexture = renderer.newRenderViewToTexture();
    if (!renderViewToTexture) {
        return;
    }
    view.mRenderToTexture.emplace();
    view.mRenderToTexture->rendererInterface = std::move(renderViewToTexture);
    view.mRenderToTexture->invalidArea = IRenderViewToTexture::InvalidArea::Full{};
    view.mRedrawRequested = false;
    view.redraw();
}

void IRenderViewToTexture::InvalidArea::addRectangle(ARect<int> rhs) {
    if (empty()) {
        mUnderlying = Rectangles{};
    }
    if (auto rectanglez = rectangles()) {
        rectanglez->erase(std::remove_if(rectanglez->begin(), rectanglez->end(), [&](const auto& lhs) {
            if (lhs.isIntersects(rhs)) {
                rhs = {
                    .p1 = glm::min(lhs.p1, lhs.p2, rhs.p1, rhs.p2),
                    .p2 = glm::max(lhs.p1, lhs.p2, rhs.p1, rhs.p2),
                };
                return true;
            }
            return false;
        }), rectanglez->end());
        if (!rectanglez->full()) {
            *rectanglez << rhs;
            return;
        }
    }
    mUnderlying = Full{};
}
