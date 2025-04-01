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
