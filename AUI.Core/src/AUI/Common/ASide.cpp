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

#include "ASide.h"

glm::vec2 ASides::getNormalVector(ASide s)
{
	switch (s)
	{
	case S_TOP:
		return { 0, -1 };
	case S_BOTTOM:
		return { 0, 1 };
	case S_LEFT:
		return { -1, 0 };
	case S_RIGHT:
		return { 1, 0 };
	case S_NONE:
		return {};
	}
	glm::ivec2 result;

	if (s & S_LEFT)
	{
		result += glm::vec2(-1, 0);
	}
	if (s & S_RIGHT)
	{
		result += glm::vec2(1, 0);
	}
	if (s & S_TOP)
	{
		result += glm::vec2(0, -1);
	}
	if (s & S_BOTTOM)
	{
		result += glm::vec2(0, 1);
	}
	
	return result;
}
