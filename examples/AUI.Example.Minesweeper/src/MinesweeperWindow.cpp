#include "MinesweeperWindow.h"

#include "CellView.h"
#include "NewGameWindow.h"
#include "AUI/Layout/AGridLayout.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AStackedLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/Platform/AMessageBox.h"
#include "AUI/Util/Random.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ALabel.h"
#include "AUI/View/ASpacer.h"
#include "AUI/View/ATextField.h"
#include "AUI/View/ANumberPicker.h"

MinesweeperWindow::MinesweeperWindow(): CustomCaptionWindow(u8"Сапёр", 1000, 1000)
{
	AVIEW_CSS;

	auto toolbox = _new<AViewContainer>();
	toolbox->addCssName(".grid");
	toolbox->setLayout(_new<AHorizontalLayout>());
	toolbox->addView(_new<ASpacer>());
	auto newGame = _new<AButton>(u8"Новая игра...");
	AObject::connect(newGame->clicked, this, &MinesweeperWindow::newGame);
	toolbox->addView(newGame);
	toolbox->addView(_new<ASpacer>());
	addView(toolbox);

	auto c = _new<AViewContainer>();
	c->setExpanding({ 1, 1 });
	c->setLayout(_new<AStackedLayout>());
	mGrid = _new<AViewContainer>();
	mGrid->addCssName(".grid");
	c->addView(mGrid);
	addView(c);
	
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

		Random r;
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
				emit customCssPropertyChanged;
				redraw();
				AMessageBox::show(this, u8"Ты лох!", u8"ТЫ ПРОДУЛ!!! АЗЗАЗАЗАЗАЗЗА");

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
			AMessageBox::show(this, u8"Ты победил!", u8"Respect +");
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
	CustomCaptionWindow::getCustomCssAttributes(map);
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
	mGrid->setLayout(_new<AGridLayout>(columns, rows));
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
				// посчитаем флаги вокруг.
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
					// открываем ячейки.

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
		});
		mGrid->addView(cell);
	}

	pack();
}
