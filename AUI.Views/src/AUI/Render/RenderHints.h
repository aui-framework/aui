#pragma once

#include <glm/glm.hpp>
#include "AUI/Views.h"
#include "AUI/Common/AColor.h"

namespace RenderHints
{
	class API_AUI_VIEWS PushAntialiasing
	{
	public:
		PushAntialiasing();
		~PushAntialiasing();
	};
	class API_AUI_VIEWS PushMatrix
	{
	private:
		glm::mat4 mStored;
		
	public:
		PushMatrix();
		~PushMatrix();
	};
	class API_AUI_VIEWS PushColor
	{
	private:
		AColor mStored;
		
	public:
		PushColor();
		~PushColor();
	};
	class API_AUI_VIEWS PushState: PushColor, PushMatrix
	{
	};
};
