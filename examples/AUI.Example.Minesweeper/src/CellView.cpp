// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include "CellView.h"

#include "AUI/Image/Drawables.h"
#include "AUI/Render/ARender.h"

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