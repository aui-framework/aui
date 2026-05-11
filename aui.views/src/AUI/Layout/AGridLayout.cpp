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

#include "AGridLayout.h"

int AGridLayout::indexOf(_<AView> view)
{
	int counter = 0;
	for (auto& i : mCells)
	{
		if (i.view == view)
		{
			return counter;
		}
		++counter;
	}
	return -1;
}

int& AGridLayout::indexAt(int x, int y)
{
	return mIndices[y * mCellsX + x];
}

AVector<_<AView>> AGridLayout::getRow(int row)
{
	ASet<int> indices;
	AVector<_<AView>> result;

	for (auto i : mIndices)
	{
		if (i != -1) {
			if (mCells[i].y == row) {
				if (!indices.contains(i))
				{
					indices << i;
					result << mCells[i].view;
				}
			}
		}
	}
	
	return result;
}

AVector<_<AView>> AGridLayout::getColumn(int column)
{
	ASet<int> indices;
	AVector<_<AView>> result;

	for (auto i : mIndices)
	{
		if (i != -1) {
			if (mCells[i].x == column) {
				if (!indices.contains(i))
				{
					indices << i;
					result << mCells[i].view;
				}
			}
		}
	}

	return result;
}

AGridLayout::AGridLayout(int cellsX, int cellsY): mCellsX(cellsX), mCellsY(cellsY)
{
	mIndices.resize(cellsX * cellsY, -1);
}

void AGridLayout::layout(int x, int y, int width, int height)
{
	float cellWidth = static_cast<float>(width) / mCellsX;
	float cellHeight = static_cast<float>(height) / mCellsY;

	for (auto& v : mCells)
	{
		auto margins = v.view->getMargin();

		auto getPos = [&](int vX, int vY) -> std::tuple<float, float>
		{
			return { x + vX * cellWidth,
				y + vY * cellHeight };
		};
		
		auto [posX1, posY1] = getPos(v.x, v.y);
		auto [posX2, posY2] = getPos(v.x + 1, v.y + 1);

		posX1 = glm::round(posX1);
		posY1 = glm::round(posY1);
		posX2 = glm::round(posX2);
		posY2 = glm::round(posY2);
		
		v.view->layout(posX1 + margins.left, posY1 + margins.top,
			posX2 - posX1 - margins.horizontal(), posY2 - posY1 - margins.vertical());
	}
}

void AGridLayout::addView(const _<AView>& view, AOptional<size_t> index) {
	if (mCurrentIndex < mIndices.size())
	{
		addView(view, mCurrentIndex % mCellsX, mCurrentIndex / mCellsX);
		mCurrentIndex += 1;
	}
}

void AGridLayout::addView(const _<AView>& view, int x, int y)
{
	auto& index = indexAt(x, y);
	if (index != -1)
	{
		throw std::runtime_error("the cell at " + std::to_string(x) + " " + std::to_string(y) + " is already taken");
	}
	index = mCells.size();
	mCells << GridCell{view, x, y};
    requestLayout();
}

void AGridLayout::removeView(aui::no_escape<AView> view, size_t index) {
    for (auto& i : mIndices)
    {
        if (i == index)
        {
            i = -1;
        }
    }
	mCells.removeAt(index);
    requestLayout();
}

glm::ivec2 AGridLayout::onIntrinsicMeasure(AConstraints constraints)
{
    const int childWidth = mCellsX == 0 ? 0 : constraints.maxInline / mCellsX;
    const int childHeight = mCellsY == 0 ? 0 : constraints.maxBlock / mCellsY;
    glm::ivec2 result = {};
    for (int y = 0; y < mCellsY; ++y)
	{
		int rowWidth = 0;
        int rowHeight = 0;
		for (auto& view : getRow(y))
		{
            auto margins = view->getMargin().occupiedSize();
            auto measured = view->measure({
                .minInline = 0,
                .maxInline = std::max(0, childWidth - margins.x),
                .minBlock = 0,
                .maxBlock = std::max(0, childHeight - margins.y),
            });
			rowWidth = glm::max(rowWidth, measured.x + margins.x);
            rowHeight = glm::max(rowHeight, measured.y + margins.y);
		}
		result.x = glm::max(result.x, rowWidth * mCellsX);
        result.y += rowHeight;
	}
	return result;
}

AMinMaxAxis AGridLayout::onComputeIntrinsicMinMaxAxis(int) {
    AMinMaxAxis result;
    for (const auto& cell : mCells) {
        if (!(cell.view->getVisibility() & Visibility::FLAG_CONSUME_SPACE)) {
            continue;
        }
        const auto minMax = cell.view->computeMinMaxAxis();
        result.min = glm::max(result.min, minMax.min + cell.view->getMargin().horizontal());
        result.max = glm::max(result.max, minMax.max + cell.view->getMargin().horizontal());
    }
    result.min *= mCellsX;
    result.max *= mCellsX;
    return result;
}

AVector<_<AView>> AGridLayout::getAllViews() {
    return { mCells.begin(), mCells.end() };
}
