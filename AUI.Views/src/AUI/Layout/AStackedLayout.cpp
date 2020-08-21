#include "AStackedLayout.h"


void ::AStackedLayout::onResize(int x, int y, int width, int height)
{
	for (auto& v : mViews)
	{
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
		v->setGeometry(finalX, finalY, finalWidth, finalHeight);
	}
}
int ::AStackedLayout::getMinimumWidth()
{
	int m = 0;
	for (auto& v : mViews)
		m = glm::max(v->getMinimumWidth(), m);
	return m;
}

int ::AStackedLayout::getMinimumHeight()
{
	int m = 0;
	for (auto& v : mViews)
		m = glm::max(v->getMinimumHeight(), m);
	return m;
}
