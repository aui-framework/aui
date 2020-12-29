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

