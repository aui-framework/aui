//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
