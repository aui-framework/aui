#include "SvgDrawable.h"

#include <glm/gtc/matrix_transform.hpp>


#include "AUI/Render/Render.h"
#include "AUI/Render/RenderHints.h"

void SvgDrawable::draw(const glm::ivec2& size)
{
	RenderHints::PushState x;
	Render::instance().setFill(Render::FILL_SOLID_TRANSFORM);
	Render::instance().setTransform(glm::scale(glm::mat4(1.f), glm::vec3{ size.x, size.y, 1.f }));

	for (auto& e : mDrawList)
	{
		e();
	}
}

glm::ivec2 SvgDrawable::getSizeHint()
{
	return glm::vec2(mViewBox.z, mViewBox.w) - glm::vec2(mViewBox.x, mViewBox.y);
}

bool SvgDrawable::isDpiDependent() const
{
	return true;
}
