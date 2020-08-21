#pragma once
#include "AUI/Util/EnumUtil.h"


ENUM_FLAG(FieldCell)
{
	F_HAS_BOMB = 1,
		F_HAS_FLAG = 4,
		F_OPEN = 2,
		F_RED_BG = 8,
		F_DONT_PLANT_BOMB_HERE = 16
};
