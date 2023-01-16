// AUI Framework - Declarative UI toolkit for modern C++17
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

#include "ASide.h"

glm::vec2 ASides::getNormalVector(ASide s)
{
	switch (s)
	{
	case ASide::TOP:
		return { 0, -1 };
	case ASide::BOTTOM:
		return { 0, 1 };
	case ASide::LEFT:
		return { -1, 0 };
	case ASide::RIGHT:
		return { 1, 0 };
	case ASide::NONE:
		return {};
	}
	glm::ivec2 result;

	if (!!(s & ASide::LEFT))
	{
		result += glm::vec2(-1, 0);
	}
	if (!!(s & ASide::RIGHT))
	{
		result += glm::vec2(1, 0);
	}
	if (!!(s & ASide::TOP))
	{
		result += glm::vec2(0, -1);
	}
	if (!!(s & ASide::BOTTOM))
	{
		result += glm::vec2(0, 1);
	}
	
	return result;
}
