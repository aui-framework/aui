// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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
 * @brief Pointing method press event.
 */
struct APointerPressedEvent {
    /**
     * @brief Where does the event occurred.
     */
    glm::vec2 position;

    /**
     * @brief Which button of the pointing device is triggered the event (AInput::LBUTTON if not present) or finger.
     */
    APointerIndex pointerIndex;

    /**
     * @brief pointerIndex treated as mouse button.
     * @details
     * = LBUTTON if finger event, pointerIndex.button() otherwise.
     */
    AInput::Key asButton = pointerIndex.button().valueOr(AInput::LBUTTON);
};


using APointerDoubleClickedEvent = APointerPressedEvent;