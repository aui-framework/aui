#pragma once

#include "AUI/Views.h"
#include <glm/glm.hpp>
#include <AUI/IO/APath.h>
#include <AUI/Thread/AFuture.h>

enum class ACursor;

namespace ADesktop
{
	API_AUI_VIEWS glm::ivec2 getMousePosition();
	API_AUI_VIEWS void setMousePos(const glm::ivec2& pos);
	API_AUI_VIEWS _<AFuture<APath>> browseForFolder();
}
