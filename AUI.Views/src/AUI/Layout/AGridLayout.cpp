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

void AGridLayout::onResize(int x, int y, int width, int height)
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
		
		v.view->setGeometry(posX1 + margins.left, posY1 + margins.top,
			posX2 - posX1 - margins.horizontal(), posY2 - posY1 - margins.vertical());
	}
}

void AGridLayout::addView(const _<AView>& view)
{
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
}

void AGridLayout::removeView(const _<AView>& view)
{
	auto index = indexOf(view);

	if (index != -1) {
		for (auto& i : mIndices)
		{
			if (i == index)
			{
				i = -1;
			}
		}
	}
	mCells.removeAt(index);
}

int AGridLayout::getMinimumWidth()
{
	int min = 0;
	for (int y = 0; y < mCellsY; ++y)
	{
		int minForRow = 0;
		for (auto& view : getRow(y))
		{
			minForRow = glm::max(int(view->getMinimumWidth() + view->getMargin().horizontal()), minForRow);
		}
		min = glm::max(minForRow * mCellsX, min);
	}
	return min;
}

int AGridLayout::getMinimumHeight()
{
	int min = 0;
	for (int x = 0; x < mCellsX; ++x)
	{
		int minForColumn = 0;
		for (auto& view : getColumn(x))
		{
			minForColumn = glm::max(int(view->getMinimumHeight() + view->getMargin().vertical()), minForColumn);
		}
		min = glm::max(minForColumn * mCellsY, min);
	}
	return min;
}
