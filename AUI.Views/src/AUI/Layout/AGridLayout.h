#pragma once
#include "ALayout.h"

class API_AUI_VIEWS AGridLayout: public ALayout
{
private:
	int mCurrentIndex = 0;
	int mCellsX, mCellsY;

	struct GridCell
	{
		_<AView> view;
		int x, y;
	};
	AVector<GridCell> mCells;
	AVector<int> mIndices;


    int& indexAt(int x, int y);

	AVector<_<AView>> getRow(int row);
	AVector<_<AView>> getColumn(int column);
public:
	AGridLayout(int cellsX, int cellsY);
	virtual ~AGridLayout() = default;

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

