/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CellView.h"

#include "AUI/Image/Drawables.h"
#include "AUI/Render/IRenderer.h"

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


void CellView::render(ARenderContext context)
{
	if (mCell != mCellValueCopy)
	{
		recompileCSS();
		mCellValueCopy = mCell;
	}
	AView::render(context);

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
			
			ctx.render.drawString(getWidth() / 2, (getHeight() - fs.size) / 2 - 3, AString::number(count), fs);
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