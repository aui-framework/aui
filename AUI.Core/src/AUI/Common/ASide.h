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
