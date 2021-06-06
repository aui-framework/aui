/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include <glm/glm.hpp>


#include "AUI/Core.h"
#include "AUI/Util/EnumUtil.h"

/**
 * Describes sides of the rectangle.
 */
ENUM_FLAG(ASide)
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
}
