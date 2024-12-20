﻿/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "MinesweeperWindow.h"

#include <AUI/Util/UIBuildingHelpers.h>
#include "CellView.h"
#include "NewGameWindow.h"
#include "AUI/Platform/AMessageBox.h"
#include "AUI/Util/ARandom.h"

MinesweeperWindow::MinesweeperWindow(): AWindow("Minesweeper", 100_dp, 100_dp)
{
    // root layout manager
    setLayout(std::make_unique<AVerticalLayout>());

    // top frame
    addView(_container<AHorizontalLayout>({
        // center with two spacers
        _new<ASpacerExpanding>(),
        // create a button and assign a slot in place
        _new<AButton>("New game...").connect(&AButton::clicked, me::newGame),
        _new<ASpacerExpanding>(),
    }) << ".frame");
    // ^^^^^^^^^^^ add a ASS class (see Style.cpp)

    // game area
    // we will use a wrapper with stacked layout to center our game area
    addView(_container<AStackedLayout>({
        // also assign ".frame" ASS class in place
        mGrid = _new<AViewContainer>() << ".frame"
    }));


	beginGame(10, 10, 20);
} 

void MinesweeperWindow::openCell(int x, int y, bool doGameLoseIfBomb)
{
	if (mDead)
	{
		//beginGame(16, 10);
		return;
	}
	FieldCell& c = fieldAt(x, y);

	if (!mBombsPlanted)
	{
		for (int i = -1; i <= 1; ++i)
		{
			for (int j = -1; j <= 1; ++j)
			{
				if (isValidCell(x + i, y + j))
					fieldAt(x + i, y + j) |= F_DONT_PLANT_BOMB_HERE;
			}
		}
		
		mBombsPlanted = true;

		ARandom r;
		for (int i = 0; i < mBombs;)
		{
			int x = r.nextInt() % mFieldColumns;
			int y = r.nextInt() % mFieldRows;
						
			if (fieldAt(x, y) == 0) {
				fieldAt(x, y) |= F_HAS_BOMB;
				++i;
			}
		}
	}
	else if (c & (F_OPEN | F_HAS_FLAG))
		return;
	{
		if (c & F_HAS_BOMB)
		{
			if (doGameLoseIfBomb) {
				c |= F_RED_BG;
				mDead = true;
				emit customCssPropertyChanged();
				redraw();
				AMessageBox::show(this, "You lost!", "You lost! Ahahahhaa!");

				//Window w;
				//w.setDialog(true);
				//w.loop();
				//beginGame(32, 20);
			}
			return;
		}
		c |= F_OPEN;
		mOpenedCells += 1;

		int bombCount = bombsAround(x, y);
		c |= FieldCell(bombCount << 16);
		
		if (mOpenedCells + mBombs == mFieldRows * mFieldColumns)
		{
			redraw();
			AMessageBox::show(this, "You won!", "Respect +");
		}


		if (bombCount == 0)
		{
			for (int i = -1; i <= 1; ++i)
			{
				for (int j = -1; j <= 1; ++j)
				{
					if (!(i == 0 && j == 0))
					{
						int cellX = x + i;
						int cellY = y + j;
						if (isValidCell(cellX, cellY))
						{
							openCell(cellX, cellY, false);
						}
					}
				}
			}
		}
	}
}

int MinesweeperWindow::bombsAround(int x, int y)
{
	int count = 0;
	for (int i = -1; i <= 1; ++i)
	{
		for (int j = -1; j <= 1; ++j)
		{
			if (!(i == 0 && j == 0))
			{
				int cellX = x + i;
				int cellY = y + j;

				if (isValidCell(cellX, cellY))
				{
					if (fieldAt(cellX, cellY) & F_HAS_BOMB)
					{
						count += 1;
					}
				}

			}
		}
	}
	return count;
}

void MinesweeperWindow::newGame()
{
	_new<NewGameWindow>(this)->show();
}

void MinesweeperWindow::getCustomCssAttributes(AMap<AString, AVariant>& map)
{
    AWindow::getCustomCssAttributes(map);
	if (mDead)
	{
		map["dead"] = true;
	}
}


void MinesweeperWindow::beginGame(int columns, int rows, int bombs)
{
	mOpenedCells = 0;
	mBombs = bombs;
	mFieldColumns = columns;
	mFieldRows = rows;
	mDead = false;
	mBombsPlanted = false;
	mGrid->setLayout(std::make_unique<AGridLayout>(columns, rows));
	mField.clear();
	mField.resize(columns * rows);
	
	for (int i = 0; i < columns * rows; ++i)
	{
		int x = i % columns;
		int y = i / columns;
		auto cell = _new<CellView>(fieldAt(x, y));
		
		connect(cell->clicked, this, [&, x, y]()
		{
			openCell(x, y, true);
            flagRedraw();
		});
		connect(cell->clickedRight, this, [&, x, y]()
		{
			auto& c = fieldAt(x, y);
			if (!(c & F_OPEN))
			{
				c ^= F_HAS_FLAG;
			}
		});
		connect(cell->doubleClicked, this, [&, x, y]()
		{
			if (fieldAt(x, y) & F_OPEN) {
				// count for flags around.
				unsigned flagCount = 0;
				for (int i = -1; i <= 1; ++i)
				{
					for (int j = -1; j <= 1; ++j)
					{
						if (!(i == 0 && j == 0) && isValidCell(x + i, y + j))
						{
							if (fieldAt(x + i, y + j) & F_HAS_FLAG)
							{
								flagCount += 1;
							}
						}
					}
				}

				if (flagCount == (fieldAt(x, y) >> 16))
				{
					// open cells.

					for (int i = -1; i <= 1; ++i)
					{
						for (int j = -1; j <= 1; ++j)
						{
							if (!(i == 0 && j == 0))
							{
								if (isValidCell(x + i, y + j))
									openCell(x + i, y + j, true);
							}
						}
					}
				}
			}
            flagRedraw();
		});
		mGrid->addView(cell);
	}

	pack();
}