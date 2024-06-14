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
