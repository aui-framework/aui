/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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

ENUM_FLAG(ASide)
{
	S_NONE = 0,
	S_LEFT = 1,
	S_RIGHT = 2,
	S_TOP = 4,
	S_BOTTOM = 8,

	S_LEFT_RIGHT = S_LEFT | S_RIGHT,
	S_TOP_BOTTOM = S_TOP | S_BOTTOM,

	S_CORNER_TOPLEFT = S_LEFT | S_TOP,
	S_CORNER_TOPRIGHT = S_RIGHT | S_TOP,
	S_CORNER_BOTTOMLEFT = S_LEFT | S_BOTTOM,
	S_CORNER_BOTTOMRIGHT = S_RIGHT | S_BOTTOM,

	S_ALL = S_LEFT | S_RIGHT | S_TOP | S_BOTTOM,

	S_EXCLUDE_LEFT = S_ALL & ~S_LEFT,
	S_EXCLUDE_RIGHT = S_ALL & ~S_RIGHT,
	S_EXCLUDE_TOP = S_ALL & ~S_TOP,
	S_EXCLUDE_BOTTOM = S_ALL & ~S_BOTTOM,
};

namespace ASides
{
	API_AUI_CORE glm::vec2 getNormalVector(ASide s);
}
