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

/*
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
*/