// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
