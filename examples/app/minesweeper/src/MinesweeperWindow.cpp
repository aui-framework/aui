/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
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

using namespace declarative;

MinesweeperWindow::MinesweeperWindow() : AWindow("Minesweeper", 100_dp, 100_dp) {
    setContents(Vertical {
      Horizontal {
        Centered::Expanding {
          _new<AButton>("New game...").connect(&AButton::clicked, me::newGame),
        },
      },
      _container<AStackedLayout>(
          { // also assign ".frame" ASS class in place
            mGrid = _new<AViewContainer>() << ".frame" }),
    } AUI_OVERRIDE_STYLE { LayoutSpacing { 4_dp } });

    beginGame(10, 10, 20);
}

void MinesweeperWindow::openCell(int x, int y, bool doGameLoseIfBomb) {
    if (mReveal) {
        // beginGame(16, 10);
        return;
    }
    FieldCell& c = fieldAt(x, y);

    if (!mBombsPlanted) {
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                if (isValidCell(x + i, y + j))
                    fieldAt(x + i, y + j) |= FieldCell::DONT_PLANT_BOMB_HERE;
            }
        }

        mBombsPlanted = true;

        ARandom r;
        for (int i = 0; i < mBombs;) {
            int x = r.nextInt() % mFieldColumns;
            int y = r.nextInt() % mFieldRows;

            if (fieldAt(x, y) == FieldCell::EMPTY) {
                fieldAt(x, y) |= FieldCell::HAS_BOMB;
                ++i;
            }
        }
    } else if (bool(c & (FieldCell::OPEN | FieldCell::HAS_FLAG))) {
        return;
    }

    if (bool(c & FieldCell::HAS_BOMB)) {
        if (doGameLoseIfBomb) {
            c |= FieldCell::RED_BG;
            mReveal = true;
            emit customCssPropertyChanged();
            redraw();
            AMessageBox::show(this, "You lost!", "You lost! Ahahahhaa!");
        }
        return;
    }
    c |= FieldCell::OPEN;
    mOpenedCells += 1;

    int bombCount = countBombsAround(x, y);
    c |= FieldCell(bombCount << 16);
    updateCellViewStyle(x, y);

    if (mOpenedCells + mBombs == mFieldRows * mFieldColumns) {
        mReveal = true;
        emit customCssPropertyChanged();
        redraw();
        AMessageBox::show(this, "You won!", "Respect +");
    }

    if (bombCount == 0) {
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                if (!(i == 0 && j == 0)) {
                    int cellX = x + i;
                    int cellY = y + j;
                    if (isValidCell(cellX, cellY)) {
                        openCell(cellX, cellY, false);
                    }
                }
            }
        }
    }
}
void MinesweeperWindow::updateCellViewStyle(int x, int y) const {
    AUI_EMIT_FOREIGN(mGrid->getViews()[y * mFieldColumns + x], customCssPropertyChanged);
}

int MinesweeperWindow::countBombsAround(int x, int y) {
    int count = 0;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (!(i == 0 && j == 0)) {
                int cellX = x + i;
                int cellY = y + j;

                if (isValidCell(cellX, cellY)) {
                    if (bool(fieldAt(cellX, cellY) & FieldCell::HAS_BOMB)) {
                        count += 1;
                    }
                }
            }
        }
    }
    return count;
}

void MinesweeperWindow::newGame() { _new<NewGameWindow>(this)->show(); }


/// [beginGame]
void MinesweeperWindow::beginGame(int columns, int rows, int bombs) {
    mOpenedCells = 0;
    mBombs = bombs;
    mFieldColumns = columns;
    mFieldRows = rows;
    mReveal = false;
    mBombsPlanted = false;
    mGrid->setLayout(std::make_unique<AGridLayout>(columns, rows));
    mField.clear();
    mField.resize(columns * rows);

    for (int i = 0; i < columns * rows; ++i) {
        int x = i % columns;
        int y = i / columns;
        auto cell = _new<CellView>(fieldAt(x, y));

        setupEventHandlers(x, y, cell);
        mGrid->addView(cell);
    }

    pack();
}
/// [beginGame]

void MinesweeperWindow::setupEventHandlers(int x, int y, const _<CellView>& cell) {
    /// [clicked]
    connect(cell->clicked, this, [&, x, y]() {
        if (mReveal) {
            return;
        }
        openCell(x, y, true);
    });
    /// [clicked]

    /// [clickedRight]
    connect(cell->clickedRight, this, [&, x, y]() {
        if (mReveal) {
            return;
        }
        auto& c = fieldAt(x, y);
        if (!(c & FieldCell::OPEN)) {
            c ^= FieldCell::HAS_FLAG;
            updateCellViewStyle(x, y);
        }
    });
    /// [clickedRight]

}
