/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
