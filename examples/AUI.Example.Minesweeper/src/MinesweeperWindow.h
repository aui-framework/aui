// AUI Framework - Declarative UI toolkit for modern C++17
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
#include "FieldCell.h"
#include "AUI/Platform/ACustomCaptionWindow.h"


class MinesweeperWindow: public AWindow
{
private:
	int mFieldColumns;
	int mFieldRows;
	bool mBombsPlanted = false;
	int mBombs;
	int mOpenedCells;
	
	_<AViewContainer> mGrid;
	AVector<FieldCell> mField;

	void openCell(int x, int y, bool doGameLoseIfBomb);
	int bombsAround(int x, int y);
	bool isValidCell(int x, int y)
	{
		return x >= 0 && x < mFieldColumns && y >= 0 && y < mFieldRows;
	}
	FieldCell& fieldAt(int x, int y)
	{
		return mField[mFieldColumns * y + x];
	}

	bool mDead = false;

	void newGame();
	
public:
	void getCustomCssAttributes(AMap<AString, AVariant>& map) override;
	void beginGame(int columns, int rows, int bombs);
	MinesweeperWindow();
};
