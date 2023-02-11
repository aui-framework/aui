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


#include "AUI/Core.h"
#include "AUI/Util/EnumUtil.h"

/**
 * @brief Describes sides of a 2D rectangle.
 * @ingroup core
 * @see ASides
 */
AUI_ENUM_FLAG(ASide)
{
	NONE = 0,
	LEFT = 1,
	RIGHT = 2,
	TOP = 4,
	BOTTOM = 8,

	LEFT_RIGHT = ASide::LEFT | ASide::RIGHT,
	TOP_BOTTOM = ASide::TOP | ASide::BOTTOM,

	CORNER_TOPLEFT = ASide::LEFT | ASide::TOP,
	CORNER_TOPRIGHT = ASide::RIGHT | ASide::TOP,
	CORNER_BOTTOMLEFT = ASide::LEFT | ASide::BOTTOM,
	CORNER_BOTTOMRIGHT = ASide::RIGHT | ASide::BOTTOM,

	ALL = ASide::LEFT | ASide::RIGHT | ASide::TOP | ASide::BOTTOM,

	EXCEPT_LEFT = ALL & ~ASide::LEFT,
	EXCEPT_RIGHT = ALL & ~ASide::RIGHT,
	EXCEPT_TOP = ALL & ~ASide::TOP,
	EXCEPT_BOTTOM = ALL & ~ASide::BOTTOM,
};
namespace ASides
{
	API_AUI_CORE glm::vec2 getNormalVector(ASide s);

    constexpr ASide inverse(const ASide& s) {
        switch (s) {
            case ASide::LEFT: return ASide::RIGHT;
            case ASide::RIGHT: return ASide::RIGHT;
            case ASide::TOP: return ASide::TOP;
            case ASide::BOTTOM: return ASide::BOTTOM;
            default: return ASide::NONE;
        }
    }
}
