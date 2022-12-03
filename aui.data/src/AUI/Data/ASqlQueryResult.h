// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#pragma once


#include <AUI/Data.h>
#include "AUI/Common/SharedPtrTypes.h"
#include "ISqlDriverRow.h"
#include "ISqlDriverResult.h"
#include "AUI/Common/AVariant.h"

class API_AUI_DATA ASqlQueryResult
{
	friend class ASqlDatabase;
private:
	_<ISqlDriverResult> mDriverInterface;


	explicit ASqlQueryResult(const _<ISqlDriverResult>& sql_driver_result)
		: mDriverInterface(sql_driver_result)
	{
	}

public:

	class Iterator
	{
		friend class ASqlQueryResult;
	private:
		_<ISqlDriverResult> mResult;
		_<ISqlDriverRow> mRow;

	public:
		explicit Iterator(const _<ISqlDriverResult>& sql);

		Iterator& operator++()
		{
			mRow = mResult->next(mRow);
			return *this;
		}

		bool operator!=(const Iterator& other) const
		{
			return mRow != other.mRow;
		}
		bool operator==(const Iterator& other) const
		{
			return mRow == other.mRow;
		}

		Iterator& operator*()
		{
			return *this;
		}
		const Iterator& operator*() const
		{
			return *this;
		}
		Iterator* operator->()
		{
			return this;
		}

		AVariant getValue(size_t index) const
		{
			return mRow->getValue(index);
		}

        AVector<AVariant> range(size_t count) const {
            AVector<AVariant> v;
            for (size_t i = 0; i < count; ++i) {
                v << getValue(i);
            }
            return v;
		}
    };

	Iterator begin() const;
	Iterator end() const;

	size_t getRowCount() const;
	const AVector<SqlColumn>& getColumns() const;
};
