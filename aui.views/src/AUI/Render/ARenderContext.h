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

#pragma once

#include <AUI/Core.h>
#include <glm/glm.hpp>
#include <AUI/Geometry2D/ARect.h>
#include <AUI/Common/AStaticVector.h>

class IRenderer;

/**
 * @brief Render context passed to AView::render.
 * @details
 * View containers are responsible to modify (by withShiftedPosition) and passthrough clip optimization context in order
 * to determine which views do not affect actual renderbuffer image and thus should not be rendered either. It's
 * applicable for AScrollArea in the first place, or any other container with AOverflow::HIDDEN and a possibility to
 * either direct or indirect children to run out (render outside) of that container.
 *
 * View containers are also responsible to skip rendering of views that are outside of the clipping.
 *
 * ARenderContext is useful only for container views.
 * 
 * ARenderContext passed to the view (possibly AViewContainer) describes an axis aligned bounding box relative
 * to it's coordinate space (position).
 *
 * Root (window) and AOverflow::HIDDEN containers should create ARenderContext with position = {0, 0} and
 * size = it's size. Other containers should not affect ARenderContext and pass it to it's children as is, in
 * exception to position, which should be subtracted by view's position.
 *
 * See UIRenderOptimizationTest for tests.
 */
struct API_AUI_VIEWS ARenderContext
{
    using Rectangles = AStaticVector<ARect<int>, 8>;

    /**
     * @brief Axis aligned bounding boxes where the rendering is performed in, used for optimization.
     */
    Rectangles clippingRects;
    IRenderer& render;

    bool debugLayout = false;

    void clip(ARect<int> clipping);

    [[nodiscard]]
    ARenderContext withShiftedPosition(glm::ivec2 by) const noexcept{
        auto copy = *this;
        for (auto& r : copy.clippingRects) r.translate(by);
        return copy;
    }
};
