// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <glm/glm.hpp>
#include "APointerIndex.h"

/**
 * @brief Pointing method scroll event.
 */
struct AScrollEvent {
    /**
     * @brief Where does the event occurred.
     * @details
     * For mouse, it's the position of the cursor.
     *
     * For touchscreen, it's the origin point where the scroll gesture started.
     */
    glm::vec2 origin;

    /**
     * @brief The amount of scroll
     * @details
     * This field is mutable in order to be consumable.
     */
    mutable glm::vec2 delta;

    /**
     * @brief Whether the event simulated by kinetic scroll feature or not.
     */
    bool kinetic = false;

    /**
     * @brief Pointer index for touchscreen; LBUTTON for mouse cursor.
     */
    APointerIndex pointerIndex = APointerIndex::button(AInput::LBUTTON);
};
