#pragma once
#include "FieldCell.h"
#include "AUI/Platform/ACustomCaptionWindow.h"


class MinesweeperWindow: public CustomCaptionWindow
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
