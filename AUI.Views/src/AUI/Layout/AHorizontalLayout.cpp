#include "AHorizontalLayout.h"
#include "AUI/Common/AVector.h"
#include "AUI/View/AViewContainer.h"

#undef max
#undef min


void AHorizontalLayout::onResize(int x, int y, int width, int height)
{
	if (mViews.empty())
		return;
	struct cache_t
	{
		int expanding;
		int minSpace;
	};
	AVector<cache_t> cache;
	cache.reserve(mViews.size());
	
	int sum = 0;
	int availableSpace = width;
	
	for (auto& view : mViews)
	{
		int e = view->getExpandingHorizontal();
		int minSpace = view->getMinimumWidth() + view->getMargin().horizontal();
		sum += e;
		if (e == 0)
			availableSpace -= (minSpace + mSpacing);
		cache << cache_t{e, minSpace};
	}

	sum = glm::max(sum, 1);
	
	unsigned index = 0;

	float posX = x;
	auto last = mViews.back();
	for (auto& view : mViews)
	{
		auto maxSize = view->getMaxSize();
		auto& e = cache[index++];
		auto margins = view->getMargin();

		if (view == last)
		{
			// последний элемент должен находиться идеально ровно на границе.
			int viewPosX = glm::round(posX) + margins.left;
			view->setGeometry(viewPosX, y + margins.top, width - viewPosX - margins.right, height - margins.vertical());
		}
		else {
			float viewWidth = glm::clamp(float(availableSpace * e.expanding) / sum, float(e.minSpace), float(maxSize.x));

			view->setGeometry(glm::round(posX) + margins.left, y + margins.top, viewWidth - margins.horizontal(), glm::min(height - margins.vertical(), float(maxSize.y)));
			posX += viewWidth + mSpacing;

			availableSpace += viewWidth - view->getSize().x;
		}
	}
	//assert(width == x);
}


int AHorizontalLayout::getMinimumWidth()
{
	int minWidth = -mSpacing;

	for (auto& v : mViews)
	{
		minWidth += v->getMinimumWidth() + mSpacing + v->getMargin().horizontal();
	}

	return glm::max(minWidth, 0);
}

int AHorizontalLayout::getMinimumHeight()
{
	int minHeight = 0;
	for (auto& v : mViews)
	{
		minHeight = glm::max(minHeight, int(v->getMinimumHeight() + v->getMargin().vertical()));
	}
	return minHeight;
}

void AHorizontalLayout::setSpacing(int spacing)
{
	mSpacing = spacing;
}

