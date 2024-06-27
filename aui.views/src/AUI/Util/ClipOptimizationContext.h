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

#pragma once

#include "glm/fwd.hpp"
#include <AUI/Core.h>
#include <glm/glm.hpp>


/**
 * @brief Axis aligned bounding box where the rendering is performed in, used for optimization.
 * @details
 * View containers are responsible to modify and passthrough clip optimization context in order to determine which views
 * do not affect actual renderbuffer image and thus should not be rendered either. It's applicable for AScrollArea in
 * the first place, or any other container with AOverflow::HIDDEN and a possibility to either direct or indirect
 * children to run out (render outside) of that container.
 *
 * View containers are also responsible to skip rendering of views that are outside of the clipping.
 *
 * ClipOptimizationContext is useful only for container views.
 * 
 * ClipOptmizationContext passed to the view (possibly AViewContainer) describes an axis aligned bounding box relative
 * to it's coordinate space (position).
 *
 * Root (window) and AOverflow::HIDDEN containers should create ClipOptimizationContext with position = {0, 0} and
 * size = it's size. Other containers should not affect ClipOptmizationContext and pass it to it's children as is, in 
 * exception to position, which should be subtracted by view's position.
 *
 * See UIRenderOptimizationTest for tests.
 */
struct API_AUI_VIEWS ClipOptimizationContext
{
    glm::ivec2 position;
    glm::ivec2 size;

    [[nodiscard]]
    ClipOptimizationContext withShiftedPosition(glm::ivec2 by) const noexcept{
        auto copy = *this;
        copy.position += by;
        return copy;
    }
};
