#include "AStackedLayout.h"


void ::AStackedLayout::onResize(int x, int y, int width, int height)
{
	for (auto& v : mViews)
	{
	    v->ensureAssUpdated();
		int finalX, finalY, finalWidth, finalHeight;
		if (v->getExpandingHorizontal() == 0)
		{
			finalWidth = v->getMinimumWidth();
			finalX = (width - finalWidth) / 2;
		} else
		{
			finalX = 0;
			finalWidth = width;
		}
		if (v->getExpandingVertical() == 0)
		{
			finalHeight = v->getMinimumHeight();
			finalY = (height - finalHeight) / 2;
		} else
		{
			finalY = 0;
			finalHeight = height;
		}
		v->setGeometry(finalX + x, finalY + y, finalWidth, finalHeight);
	}
}
int ::AStackedLayout::getMinimumWidth()
{
	int m = 0;
	for (auto& v : mViews)
		if (v->getVisibility() != AView::V_GONE)
			m = glm::max(int(v->getMinimumWidth() + v->getMargin().horizontal()), m);
	return m;
}

int ::AStackedLayout::getMinimumHeight()
{
	int m = 0;
	for (auto& v : mViews) {
		if (v->getVisibility() != AView::V_GONE)
			m = glm::max(int(v->getMinimumHeight() + v->getMargin().vertical()), m);
	}
	return m;
}
