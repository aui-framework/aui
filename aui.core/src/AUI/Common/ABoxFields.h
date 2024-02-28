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

/**
 * @brief Represents a rectangle fields. Useful for margin and padding around AViews.
 * @ingroup core
 */
struct ABoxFields
{
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;


    [[nodiscard]]
    glm::ivec2 leftTop() const noexcept {
        return { left, top };
    }

    [[nodiscard]]
    glm::ivec2 rightTop() const noexcept {
        return { right, top };
    }

    [[nodiscard]]
    glm::ivec2 leftBottom() const noexcept {
        return { left, bottom };
    }

    [[nodiscard]]
    glm::ivec2 rightBottom() const noexcept {
        return { right, bottom };
    }

    [[nodiscard]]
    int horizontal() const noexcept
    {
        return left + right;
    }

    [[nodiscard]]
    int vertical() const noexcept
    {
        return top + bottom;
    }
    
    [[nodiscard]]
    glm::ivec2 occupiedSize() const noexcept {
        return { horizontal(), vertical() };
    }

};
