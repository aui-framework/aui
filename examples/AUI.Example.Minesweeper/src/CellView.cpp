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

#include "CellView.h"

#include "AUI/Image/Drawables.h"
#include "AUI/Render/Render.h"

CellView::CellView(FieldCell& cell)
	: mCell(cell), mCellValueCopy(cell)
{

	
	connect(clickedButton, this, [&]()
	{
		mOpen = true;
		emit customCssPropertyChanged();
	});
}

int CellView::getContentMinimumWidth()
{
	return AView::getContentMinimumWidth();
}


void CellView::render()
{
	if (mCell != mCellValueCopy)
	{
		recompileCSS();
		mCellValueCopy = mCell;
	}
	AView::render();

	if (mCell & F_OPEN)
	{
		int count = mCell >> 16;
		if (count)
		{
			FontStyle fs;
			fs.size = getHeight() * 6 / 7;
			fs.align = TextAlign::CENTER;

			switch (count)
			{
			case 1:
				fs.color = 0x0000ffffu;
				break;
			case 2:
				fs.color = 0x008000ffu;
				break;
			case 3:
				fs.color = 0xff0000ffu;
				break;
			case 4:
				fs.color = 0x000080ffu;
				break;
			case 5:
				fs.color = 0x800000ffu;
				break;
			case 6:
				fs.color = 0x008080ffu;
				break;
			case 7:
				fs.color = 0x000000ffu;
				break;
			case 8:
				fs.color = 0x808080ffu;
				break;
			}
			
			Render::drawString(getWidth() / 2, (getHeight() - fs.size) / 2 - 3, AString::number(count), fs);
		}
	}
}

void CellView::getCustomCssAttributes(AMap<AString, AVariant>& map)
{
	AView::getCustomCssAttributes(map);
	map["open"] = bool(mCell & F_OPEN);
	map["flag"] = bool(mCell & F_HAS_FLAG);
	map["bomb"] = bool(mCell & F_HAS_BOMB);
	if (mCell & F_RED_BG)
		map["red"] = true;
}