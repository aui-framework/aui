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

#include <glm/glm.hpp>


#include "AUI/Reflect/AEnumerate.h"

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
