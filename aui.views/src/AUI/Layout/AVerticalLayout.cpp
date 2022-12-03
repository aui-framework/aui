// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "AVerticalLayout.h"
#include "AUI/Common/AVector.h"
#include "AUI/View/AViewContainer.h"

#undef max
#undef min

void AVerticalLayout::onResize(int x, int y, int width, int height)
{
    if (mViews.empty()) return;
	struct cache_t
	{
		int expanding;
		int minSpace;
	};
	AVector<cache_t> cache;
	cache.reserve(mViews.size());

	int sum = 0;
	int availableSpace = height;


	for (auto& view : mViews)
	{
        view->ensureAssUpdated();
        if (view->getVisibility() == Visibility::GONE) continue;
		int e = view->getExpandingVertical();
		int minSpace = view->getMinimumHeight(ALayoutDirection::VERTICAL);
		sum += e;
		if (e == 0 || view->getFixedSize().y != 0)
			availableSpace -= minSpace + view->getMargin().vertical() + mSpacing;
		else
			availableSpace -= view->getMargin().vertical() + mSpacing;
		cache << cache_t{ e, minSpace };
	}

    bool containsExpandingItems = sum > 0;

	sum = glm::max(sum, 1);

	unsigned index = 0;
    int posY = y;
    auto last = mViews.back();
	for (auto& view : mViews)
	{
        if (view->getVisibility() == Visibility::GONE) continue;
		auto margins = view->getMargin();
		auto maxSize = view->getMaxSize();
		auto& e = cache[index++];

        if (containsExpandingItems && view == last)
        {
            // the last element should stick right to the border.
            int viewPosY = posY + margins.top;
            int viewHeight = height - viewPosY - margins.bottom + y;
            view->setGeometry(x + margins.left,
                              viewPosY,
                              width - margins.horizontal(),
                              viewHeight);
        }
        else {
            int viewHeight = glm::clamp(availableSpace * e.expanding / sum, e.minSpace, maxSize.y);

            view->setGeometry(x + margins.left,
                              posY + margins.top,
                              glm::min(width - margins.horizontal(), maxSize.x),
                              viewHeight);

            posY += view->getSize().y + mSpacing + margins.vertical();

            availableSpace += viewHeight - view->getSize().y;
        }
	}
}


int AVerticalLayout::getMinimumWidth()
{
	int minWidth = 0;
	for (auto& v : mViews)
	{
	    if (v->getVisibility() == Visibility::GONE) continue;
		minWidth = glm::max(minWidth, int(v->getMinimumWidth(ALayoutDirection::VERTICAL) + v->getMargin().horizontal()));
	}
	return minWidth;
}

int AVerticalLayout::getMinimumHeight()
{
	int minHeight = -mSpacing;

	for (auto& v : mViews)
	{
        if (v->getVisibility() == Visibility::GONE) continue;
		minHeight += v->getMinimumHeight(ALayoutDirection::VERTICAL) + mSpacing + v->getMargin().vertical();
	}

	return glm::max(minHeight, 0);
}

void AVerticalLayout::setSpacing(int spacing)
{
	mSpacing = spacing;
}

ALayoutDirection AVerticalLayout::getLayoutDirection() {
    return ALayoutDirection::VERTICAL;
}
