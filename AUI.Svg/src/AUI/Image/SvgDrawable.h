#pragma once
#include <functional>

#include "AUI/Common/ADeque.h"
#include "AUI/Image/IDrawable.h"

class SvgDrawable: public IDrawable
{
	friend class SvgEntityVisitor;
	friend class SvgDocumentVisitor;
	friend class PolygonVisitor;
	friend class PathVisitor;
	friend class RectVisitor;
	
private:	
	glm::vec4 mViewBox;
	ADeque<std::function<void()>> mDrawList;
	
public:
	void draw(const glm::ivec2& size) override;
	glm::ivec2 getSizeHint() override;

	bool isDpiDependent() const override;
};
