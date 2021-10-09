/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
		view->ensureAssUpdated();
        if (view->getVisibility() == Visibility::GONE) continue;
		int e = view->getExpandingHorizontal();
		auto m = view->getMinimumWidth();
		int minSpace = m + view->getMargin().horizontal();
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
        if (view->getVisibility() == Visibility::GONE) continue;
		auto maxSize = view->getMaxSize();
		auto& e = cache[index++];
		auto margins = view->getMargin();
		//auto cmin = view->getMinimumWidth();
		//assert(cmin == e.minSpace - margins.horizontal());

		if (view == last)
		{
			// the last element should stick right to the border.
			int viewPosX = glm::round(posX) + margins.left;
			//assert(int(width - viewPosX - margins.right) >= e.minSpace - margins.horizontal());
			int viewWidth = width - viewPosX - margins.right;
			view->setGeometry(viewPosX, y + margins.top, viewWidth, height - margins.vertical());
		}
		else {
			float viewWidth = glm::clamp(float(availableSpace * e.expanding) / sum, float(e.minSpace), float(maxSize.x));

			view->setGeometry(glm::round(posX) + margins.left, y + margins.top, viewWidth - margins.horizontal(), glm::min(height - margins.vertical(), float(maxSize.y)));
			posX += viewWidth + mSpacing;

			availableSpace += viewWidth - (view->getSize().x + view->getMargin().horizontal());
		}
	}
	//assert(width == x);
}


int AHorizontalLayout::getMinimumWidth()
{
	int minWidth = -mSpacing;

	for (auto& v : mViews)
	{
        if (v->getVisibility() == Visibility::GONE) continue;
		minWidth += v->getMinimumWidth() + mSpacing + v->getMargin().horizontal();
	}

	return glm::max(minWidth, 0);
}

int AHorizontalLayout::getMinimumHeight()
{
	int minHeight = 0;
	for (auto& v : mViews)
	{
        if (v->getVisibility() == Visibility::GONE) continue;
	    auto h = v->getMinimumHeight();
		minHeight = glm::max(minHeight, int(h + v->getMargin().vertical()));
	}
	return minHeight;
}

void AHorizontalLayout::setSpacing(int spacing)
{
	mSpacing = spacing;
}

