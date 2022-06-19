/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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