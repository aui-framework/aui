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
	setWindowStyle(WS_DIALOG);

	setLayout(_new<AVerticalLayout>());

	// form
	addView(_form({
        {"Cells by width:"_as, mWidth = _new<ANumberPicker>() let (ANumberPicker, {
            setMin(8);
            setMax(25);
        })},
        {"Cells by height:"_as, mHeight = _new<ANumberPicker>() let (ANumberPicker, {
            setMin(8);
            setMax(25);
        })},
        {"Mines count:"_as, mMines = _new<ANumberPicker>() let (ANumberPicker, {
            setMin(8);
        })},
	}));

	// difficulty label
    addView(mDifficultyLabel = _new<ALabel>());

	// buttons
	addView(_container<AHorizontalLayout>({
	    _new<ASpacer>(),
        _new<AButton>("Start game") let(AButton, {
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
