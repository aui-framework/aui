// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#include "NewGameWindow.h"


#include "AUI/Layout/AGridLayout.h"
#include "AUI/Layout/AHorizontalLayout.h"
#include "AUI/Layout/AVerticalLayout.h"
#include "AUI/View/AButton.h"
#include "AUI/View/ALabel.h"
#include "AUI/View/ANumberPicker.h"
#include "AUI/Util/UIBuildingHelpers.h"

int gWidth = 10;
int gHeight = 10;
int gMines = 10;

void NewGameWindow::updateMinesMax()
{
	mMines->setMax(mWidth->getValue() * mHeight->getValue() - 25);
}

void NewGameWindow::updateDifficultyLabel()
{
    mMines->setMax(mWidth->getValue() * mHeight->getValue() * 3 / 4);
	int difficulty = mWidth->getValue() * mHeight->getValue() / (glm::max)(mMines->getValue(), 1);

	AString text = "Difficulty: ";
	switch (difficulty)
	{
    default:
	case 0:
	case 1:
		text += "very low";
		break;
	case 2:
	case 3:
		text += "high";
		break;
	case 4:
	case 5:
		text += "medium";
		break;
	case 6:
	case 7:
	case 8:
		text += "low";
		break;

	}
	mDifficultyLabel->setText(text);
}

NewGameWindow::NewGameWindow(MinesweeperWindow* minesweeper):
	AWindow("New game", 100, 100, minesweeper),
	mMinesweeper(minesweeper)
{
	setWindowStyle(WindowStyle::DIALOG);

	setLayout(_new<AVerticalLayout>());

	// form
	addView(_form({
        {"Cells by width:"_as, mWidth = _new<ANumberPicker>() let {
            setMin(8);
            setMax(25);
        })},
        {"Cells by height:"_as, mHeight = _new<ANumberPicker>() let {
            setMin(8);
            setMax(25);
        })},
        {"Mines count:"_as, mMines = _new<ANumberPicker>() let {
            setMin(8);
        })},
	}));

	// difficulty label
    addView(mDifficultyLabel = _new<ALabel>());

	// buttons
	addView(_container<AHorizontalLayout>({
	    _new<ASpacerExpanding>(),
        _new<AButton>("Start game") let {
            setDefault();
        }).connect(&AButton::clicked, me::begin),
        _new<AButton>("Cancel").connect(&AButton::clicked, me::close),
	}));


	mWidth->setValue(gWidth);
	mHeight->setValue(gHeight);
	
	updateMinesMax();
	
	mMines->setValue(gMines);
	
	updateDifficultyLabel();

	connect(mWidth->valueChanging, this, &NewGameWindow::updateMinesMax);
	connect(mWidth->valueChanging, this, &NewGameWindow::updateDifficultyLabel);
	connect(mHeight->valueChanging, this, &NewGameWindow::updateMinesMax);
	connect(mHeight->valueChanging, this, &NewGameWindow::updateDifficultyLabel);
	connect(mMines->valueChanging, this, &NewGameWindow::updateDifficultyLabel);
	
	pack();
}

void NewGameWindow::begin() {
    close();
    mMinesweeper->beginGame(gWidth = mWidth->getValue(),
                            gHeight = mHeight->getValue(), gMines = mMines->getValue());
}
