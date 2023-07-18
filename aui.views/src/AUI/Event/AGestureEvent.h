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
#include <variant>

/**
 * @brief Long press event.
 * @details
 * When handled, causes haptic feedback on some systems.
 */
struct ALongPressEvent {
    mutable bool handled = false;
};

struct AFingerDragEvent {
    glm::vec2 delta;
    bool kinetic = false;
};


using AGestureEvent = std::variant<ALongPressEvent, AFingerDragEvent>;