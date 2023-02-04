// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
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

#include "AAdvancedGridLayout.h"

int AAdvancedGridLayout::indexOf(_<AView> view)
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

int& AAdvancedGridLayout::indexAt(int x, int y)
{
    return mIndices[y * cellsX + x];
}

AVector<_<AView>> AAdvancedGridLayout::getRow(int row)
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

AVector<_<AView>> AAdvancedGridLayout::getColumn(int column)
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

AAdvancedGridLayout::AAdvancedGridLayout(int cellsX, int cellsY): cellsX(cellsX), cellsY(cellsY)
{
    mIndices.resize(cellsX * cellsY, -1);
}

void AAdvancedGridLayout::onResize(int x, int y, int width, int height)
{
    //float cellWidth = static_cast<float>(width) / cellsX;
    //float cellHeight = static_cast<float>(height) / cellsY;

    struct cache_t
    {
        unsigned expandingSum = 0;
        int minSize = 0;

        int finalPos = 0;
        int finalSize = 0;
    };

    AVector<cache_t> columns;
    AVector<cache_t> rows;

    columns.resize(cellsX);
    rows.resize(cellsY);



    // preparation
    for (auto& v : mCells) {
        v.view->ensureAssUpdated();
        if (v.view->getVisibility() == Visibility::GONE) continue;

        glm::ivec2 e = {v.view->getExpandingHorizontal(), v.view->getExpandingVertical()};
        auto fixed = v.view->getFixedSize();
        if (fixed.x != 0) e.x = 0;
        if (fixed.y != 0) e.y = 0;
        glm::ivec2 m = {v.view->getMinimumWidth(), v.view->getMinimumHeight(
                ALayoutDirection::NONE)};
        glm::ivec2 minSpace = m + glm::ivec2{v.view->getMargin().horizontal(), v.view->getMargin().vertical()};

        columns[v.x].expandingSum += e.x;
        rows[v.y].expandingSum    += e.y;

        columns[v.x].minSize = glm::max(columns[v.x].minSize, minSpace.x);
        rows[v.y].minSize    = glm::max(rows[v.y].minSize,    minSpace.y);

    }
    glm::ivec2 available { width, height };

    // evaluate expansion sum
    glm::ivec2 sums(0);
    for (auto& c : columns) {
        if (c.expandingSum == 0) {
            available.x -= c.minSize;
        } else {
            sums.x += c.expandingSum;
        }
    }
    for (auto& r : rows) {
        if (r.expandingSum == 0) {
            available.y -= r.minSize;
        } else {
            sums.y += r.expandingSum;
        }
    }
    sums = glm::max(sums, glm::ivec2(1));
    float pos = x;

    // evaluate final size
    for (auto& c : columns) {
        c.finalPos = glm::round(pos);
        c.finalSize = glm::round(glm::max(float(available.x * c.expandingSum) / sums.x, float(c.minSize)));

        // align right to border
        if (c.finalPos + c.finalSize > x + width) {
            c.finalSize = x + width - c.finalPos;
        }
        pos += c.finalSize + mSpacing;
    }
    pos = y;
    for (auto& r : rows) {
        r.finalPos = glm::round(pos);
        r.finalSize = glm::round(glm::max(float(available.y * r.expandingSum) / sums.y, float(r.minSize)));


        // align down to border
        if (r.finalPos + r.finalSize > y + height) {
            r.finalSize = y + height - r.finalPos;
        }
        pos += r.finalSize + mSpacing;
    }

    // update geometry
    for (auto& v : mCells) {
        auto margins = v.view->getMargin();


        auto posX   = columns[v.x].finalPos;
        auto posY   = rows   [v.y].finalPos;
        auto width  = columns[v.x].finalSize;
        auto height = rows   [v.y].finalSize;

        auto geomX = posX + margins.left;
        auto geomY = posY + margins.top;
        auto geomW = width - margins.horizontal();
        auto geomH = height - margins.vertical();
        v.view->setGeometry(geomX, geomY, geomW, geomH);
    }
}

void AAdvancedGridLayout::addView(size_t index, const _<AView>& view)
{
    if (mCurrentIndex < mIndices.size())
    {
        addView(view, mCurrentIndex % cellsX, mCurrentIndex / cellsX);
        mCurrentIndex += 1;
    }
}

void AAdvancedGridLayout::addView(const _<AView>& view, int x, int y)
{
    auto& index = indexAt(x, y);
    if (index != -1)
    {
        throw std::runtime_error("the cell at " + std::to_string(x) + " " + std::to_string(y) + " is already taken");
    }
    index = mCells.size();
    mCells << GridCell{view, x, y};
}

void AAdvancedGridLayout::removeView(size_t index, const _<AView>& view)
{
    if (index == -1) {
        index = indexOf(view);
    }

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

int AAdvancedGridLayout::getMinimumWidth()
{
    int min = -mSpacing;
    for (int x = 0; x < cellsX; ++x)
    {
        int minForColumn = 0;
        for (auto& view : getRow(x))
        {
            minForColumn = glm::max(int(view->getMinimumWidth() + view->getMargin().horizontal()), minForColumn);
        }
        min += minForColumn + mSpacing;
    }
    return min;
}

int AAdvancedGridLayout::getMinimumHeight()
{
    int min = -mSpacing;
    for (int y = 0; y < cellsY; ++y)
    {
        int minForRow = 0;
        for (auto& view : getRow(y))
        {
            minForRow = glm::max(int(view->getMinimumHeight() + view->getMargin().vertical()), minForRow);
        }
        min += minForRow + mSpacing;
    }
    return min;
}
