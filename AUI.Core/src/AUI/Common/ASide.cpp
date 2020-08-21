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
