#pragma once

#include <glm/glm.hpp>


#include "AUI/Views.h"
#include "AUI/Common/AVector.h"

namespace Triangles
{
	API_AUI_VIEWS void triangulate(const AVector<glm::vec2>& vertices, AVector<unsigned>& indices);
	API_AUI_VIEWS void optimize(AVector<glm::vec2>& vertices);
}
