#include "AVerticalLayout.h"
#include "AUI/Common/AVector.h"
#include "AUI/View/AViewContainer.h"

#undef max
#undef min

void AVerticalLayout::onResize(int x, int y, int width, int height)
{
	struct cache_t
	{
		int expanding;
		int minSpace;
	};
	AVector<cache_t> cache;
	cache.reserve(mViews.size());

	int sum = 0;
	int availableSpace = height - y;


	for (auto& view : mViews)
	{
        view->ensureCSSUpdated();
		int e = view->getExpandingVertical();
		int minSpace = view->getMinimumHeight();
		sum += e;
		if (e == 0)
			availableSpace -= minSpace + view->getMargin().vertical() + mSpacing;
		else
			availableSpace -= view->getMargin().vertical() + mSpacing;
		cache << cache_t{ e, minSpace };
	}

	sum = glm::max(sum, 1);

	unsigned index = 0;
	for (auto& view : mViews)
	{
		auto margins = view->getMargin();
		auto maxSize = view->getMaxSize();
		auto& e = cache[index++];

		int viewHeight = glm::clamp(availableSpace * e.expanding / sum, e.minSpace,	maxSize.y);


		view->setGeometry(x + margins.left, y + margins.top, glm::min(width - margins.horizontal(), float(maxSize.x)), viewHeight);
		y += view->getSize().y + mSpacing + margins.vertical();


		availableSpace += viewHeight - view->getSize().y;
	}
}


int AVerticalLayout::getMinimumWidth()
{
	int minWidth = 0;
	for (auto& v : mViews)
	{
		minWidth = glm::max(minWidth, int(v->getMinimumWidth() + v->getMargin().horizontal()));
	}
	return minWidth;
}

int AVerticalLayout::getMinimumHeight()
{
	int minHeight = -mSpacing;

	for (auto& v : mViews)
	{
		minHeight += v->getMinimumHeight() + mSpacing + v->getMargin().vertical();
	}

	return glm::max(minHeight, 0);
}

void AVerticalLayout::setSpacing(int spacing)
{
	mSpacing = spacing;
}

