﻿// AUI Framework - Declarative UI toolkit for modern C++20
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

#pragma once
#include "ALayout.h"

/**
 * @brief Grid layout with fixed-size cells.
 * @see AAdvancedGridLayout
 * @ingroup layout_managers
 */
class API_AUI_VIEWS AGridLayout: public ALayout
{
private:
	int mCurrentIndex = 0;
	int mCellsX, mCellsY;

	struct GridCell
	{
		_<AView> view;
		int x, y;
	};
	AVector<GridCell> mCells;
	AVector<int> mIndices;


    int& indexAt(int x, int y);

	AVector<_<AView>> getRow(int row);
	AVector<_<AView>> getColumn(int column);
public:
	AGridLayout(int cellsX, int cellsY);
	virtual ~AGridLayout() = default;

	_<AView> getViewAt(size_t index) {
	    return mCells.at(index).view;
	}

	void setViewAt(size_t index, _<AView> view) {
	    mCells.at(index).view = view;
	}

	void onResize(int x, int y, int width, int height) override;
	void addView(size_t index, const _<AView>& view) override;
	void addView(const _<AView>& view, int x, int y);
	void removeView(size_t index, const _<AView>& view) override;
	int getMinimumWidth() override;
	int getMinimumHeight() override;

    int indexOf(_<AView> view);
};

