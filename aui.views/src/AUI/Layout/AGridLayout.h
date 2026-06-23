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

#pragma once
#include "ALayout.h"

/**
 * @brief Grid layout with fixed-size cells.
 * @see AAdvancedGridLayout
 * @ingroup layout-managers
 */
class API_AUI_VIEWS AGridLayout: public ALayout
{
private:
	int mCurrentIndex = 0;
	int mCellsX, mCellsY;

	struct GridCell
	{
		AArc<AView> view;
		int x, y;

        operator AArc<AView>() const {
            return view;
        }
	};
	AVector<GridCell> mCells;
	AVector<int> mIndices;


    int& indexAt(int x, int y);

	AVector<AArc<AView>> getRow(int row);
	AVector<AArc<AView>> getColumn(int column);
public:
	AGridLayout(int cellsX, int cellsY);
	virtual ~AGridLayout() = default;

	AArc<AView> getViewAt(size_t index) {
	    return mCells.at(index).view;
	}

	void setViewAt(size_t index, AArc<AView> view) {
	    mCells.at(index).view = view;
	}

	void onResize(int x, int y, int width, int height) override;
	void addView(const AArc<AView>& view, int x, int y);
    void addView(const AArc<AView>& view, AOptional<size_t> index) override;
    void removeView(aui::no_escape<AView> view, size_t index) override;
    int getMinimumWidth() override;
	int getMinimumHeight() override;
    AVector<AArc<AView>> getAllViews() override;

    int indexOf(AArc<AView> view);
};

