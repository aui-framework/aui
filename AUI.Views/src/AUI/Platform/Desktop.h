#pragma once

#include "AUI/Views.h"
#include <glm/glm.hpp>

enum class ACursor;

namespace Desktop
{
	API_AUI_VIEWS glm::ivec2 getMousePos();
	API_AUI_VIEWS void setMousePos(const glm::ivec2& pos);
}
