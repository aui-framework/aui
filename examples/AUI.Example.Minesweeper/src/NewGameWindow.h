#pragma once
#include "MinesweeperWindow.h"
#include "AUI/Platform/AWindow.h"
#include "AUI/View/ANumberPicker.h"
#include "AUI/View/ALabel.h"

class NewGameWindow: public AWindow
{
private:
	MinesweeperWindow* mMinesweeper;
	_<ANumberPicker> mWidth;
	_<ANumberPicker> mHeight;
	_<ANumberPicker> mMines;
	_<ALabel> mDifficultyLabel;

	void updateMinesMax();
	void updateDifficultyLabel();
	void begin();

public:
	NewGameWindow(MinesweeperWindow* minesweeper);
};
