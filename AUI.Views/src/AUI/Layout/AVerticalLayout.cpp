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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

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
        view->ensureAssUpdated();
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

