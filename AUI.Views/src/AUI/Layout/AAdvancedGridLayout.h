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

#pragma once

#include "ALayout.h"

class API_AUI_VIEWS AAdvancedGridLayout: public ALayout {
private:
    int mCurrentIndex = 0;
    int cellsX, cellsY;
    unsigned mSpacing = 0;

    struct GridCell {
        _<AView> view;
        int x, y;
    };
    AVector<GridCell> mCells;
    AVector<int> mIndices;


    int& indexAt(int x, int y);

    AVector<_<AView>> getRow(int row);
    AVector<_<AView>> getColumn(int column);

public:

    AAdvancedGridLayout(int cellsX, int cellsY);

    virtual ~AAdvancedGridLayout() = default;

    _<AView> getViewAt(size_t index) {
        return mCells.at(index).view;
    }

    void setViewAt(size_t index, _<AView> view) {
        mCells.at(index).view = view;
    }

    void onResize(int x, int y, int width, int height) override;
    void addView(const _<AView>& view) override;

    void addView(const _<AView>& view, int x, int y);
    void removeView(const _<AView>& view) override;

    int getMinimumWidth() override;

    int getMinimumHeight() override;

    int indexOf(_<AView> view);

};

