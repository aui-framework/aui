/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
#include "ATableView.h"
#include "ALabel.h"
#include "AUI/Layout/AGridLayout.h"
#include "AUI/Model/AListModelIndex.h"

ATableView::~ATableView()
{
}


void ATableView::setModel(_<ITableModel> model)
{
	mModel = model;

	setLayout(std::make_unique<AGridLayout>(mModel->tableColumns(), mModel->tableRows()));

	for (size_t row = 0; row < mModel->tableRows(); ++row)
	{
		for (size_t column = 0; column < mModel->tableColumns(); ++column)
		{
			addView(_new<ALabel>(mModel->tableItemAt(AListModelIndex(row, column)).toString()));
		}
	}
	updateLayout();
}
*/