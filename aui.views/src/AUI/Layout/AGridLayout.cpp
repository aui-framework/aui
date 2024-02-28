// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

AVector<_<AView>> AGridLayout::getAllViews() {
    return { mCells.begin(), mCells.end() };
}
