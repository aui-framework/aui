/*
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

#include "AStackedLayout.h"


void ::AStackedLayout::onResize(int x, int y, int width, int height)
{
	for (auto& v : mViews)
	{
	    v->ensureAssUpdated();
		int finalX, finalY, finalWidth, finalHeight;
        auto margins = v->getMargin();
		if (v->getExpandingHorizontal() == 0)
		{
			finalWidth = v->getMinimumWidth() + margins.horizontal();
			finalX = (width - finalWidth) / 2;
		} else
		{
			finalX = 0;
			finalWidth = width;
		}
		if (v->getExpandingVertical() == 0)
		{
			finalHeight = v->getMinimumHeight() + margins.vertical();
			finalY = (height - finalHeight) / 2;
		} else
		{
			finalY = 0;
			finalHeight = height;
		}
		v->setGeometry(finalX + x + margins.left,
                       finalY + y + margins.top,
                       finalWidth - margins.horizontal(),
                       finalHeight - margins.vertical());
	}
}
int ::AStackedLayout::getMinimumWidth()
{
	int m = 0;
	for (auto& v : mViews)
		if (v->getVisibility() != Visibility::GONE)
			m = glm::max(int(v->getMinimumWidth() + v->getMargin().horizontal()), m);
	return m;
}

int ::AStackedLayout::getMinimumHeight()
{
	int m = 0;
	for (auto& v : mViews) {
		if (v->getVisibility() != Visibility::GONE)
			m = glm::max(int(v->getMinimumHeight() + v->getMargin().vertical()), m);
	}
	return m;
}
