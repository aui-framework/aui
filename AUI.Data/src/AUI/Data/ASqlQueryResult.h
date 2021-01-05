/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

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
