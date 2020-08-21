#pragma once
#include "FieldCell.h"
#include "AUI/View/AView.h"

class CellView: public AView
{
private:
	bool mOpen = false;
	FieldCell& mCell;
	FieldCell mCellValueCopy;
	
public:
	CellView(FieldCell& cell);


	int getContentMinimumWidth() override;
	void render() override;
	void getCustomCssAttributes(AMap<AString, AVariant>& map) override;
};
