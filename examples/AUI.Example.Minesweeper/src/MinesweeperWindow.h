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
