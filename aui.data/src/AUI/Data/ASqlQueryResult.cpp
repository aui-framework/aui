/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ASqlQueryResult.h"


ASqlQueryResult::Iterator::Iterator(const _<ISqlDriverResult>& sql): mResult(sql)
{
	if (mResult)
	{
		mRow = mResult->begin();
	}
}

ASqlQueryResult::Iterator ASqlQueryResult::begin() const
{
	return Iterator(mDriverInterface);
}

ASqlQueryResult::Iterator ASqlQueryResult::end() const
{
	return Iterator(nullptr);
}

size_t ASqlQueryResult::getRowCount() const
{
	return mDriverInterface->rowCount();
}

const AVector<SqlColumn>& ASqlQueryResult::getColumns() const
{
	return mDriverInterface->getColumns();
}
