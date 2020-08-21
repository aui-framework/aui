#include "ATableView.h"


#include "ALabel.h"
#include "AUI/Layout/AGridLayout.h"
#include "AUI/Model/AModelIndex.h"

ATableView::~ATableView()
{
}


void ATableView::setModel(_<ITableModel> model)
{
	mModel = model;

	setLayout(_new<AGridLayout>(mModel->tableColumns(), mModel->tableRows()));

	for (size_t row = 0; row < mModel->tableRows(); ++row)
	{
		for (size_t column = 0; column < mModel->tableColumns(); ++column)
		{
			addView(_new<ALabel>(mModel->tableItemAt(AModelIndex(row, column)).toString()));
		}
	}
	updateLayout();
}
