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
        unsigned available;

        int finalPos = 0;
        int finalSize = 0;
    };

    AVector<cache_t> columns;
    AVector<cache_t> rows;

    columns.resize(cellsX);
    rows.resize(cellsY);

    for (auto& r : rows) {
        r.available = height - y;
    }
    for (auto& c : columns) {
        c.available = width - x;
    }

    // подготовка
    for (auto& v : mCells) {
        v.view->ensureCSSUpdated();
        glm::ivec2 e = {v.view->getExpandingHorizontal(), v.view->getExpandingVertical()};
        glm::ivec2 m = {v.view->getMinimumWidth(), v.view->getMinimumHeight()};
        glm::ivec2 minSpace = m + glm::ivec2{v.view->getMargin().horizontal(), v.view->getMargin().vertical()};
        if (e.x == 0)
            rows[v.y].available -= minSpace.x + mSpacing;
        if (e.y == 0)
            columns[v.x].available -= minSpace.y + mSpacing;

        columns[v.x].expandingSum += e.x;
        rows[v.y].expandingSum    += e.y;

        columns[v.x].minSize = glm::max(columns[v.x].minSize, minSpace.x);
        rows[v.y].minSize    = glm::max(rows[v.y].minSize,    minSpace.y);
    }

    // подсчёт суммы расширения
    glm::ivec2 sums(0);
    for (auto& c : columns) {
        sums.x += c.expandingSum;
    }
    for (auto& r : rows) {
        sums.y += r.expandingSum;
    }
    sums = glm::max(sums, glm::ivec2(1));
    float pos = x;

    // подсчёт финального размера
    for (auto& c : columns) {
        c.finalPos = glm::round(pos);
        c.finalSize = glm::round(glm::max(float(c.available * c.expandingSum) / sums.x, float(c.minSize)));

        pos += c.finalSize + mSpacing;
    }
    pos = y;
    for (auto& r : rows) {
        r.finalPos = glm::round(pos);
        r.finalSize = glm::round(glm::max(float(r.available * r.expandingSum) / sums.y, float(r.minSize)));

        pos += r.finalSize + mSpacing;
    }

    // установка позиций
    for (auto& v : mCells) {
        auto margins = v.view->getMargin();


        auto posX   = columns[v.x].finalPos;
        auto posY   = rows   [v.y].finalPos;
        auto width  = columns[v.x].finalSize;
        auto height = rows   [v.y].finalSize;

        v.view->setGeometry(posX + margins.left, posY + margins.top,
                            width - margins.horizontal(), height - margins.vertical());
    }
}

void AAdvancedGridLayout::addView(const _<AView>& view)
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

void AAdvancedGridLayout::removeView(const _<AView>& view)
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
