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
#include "ISqlDriverRow.h"
#include "SqlTypes.h"
#include <AUI/Common/AVector.h>

class ISqlDriverResult
{
public:
	virtual ~ISqlDriverResult() = default;
	virtual const AVector<SqlColumn>& getColumns() = 0;
	virtual size_t rowCount() = 0;
	virtual _<ISqlDriverRow> begin() = 0;
	virtual _<ISqlDriverRow> next(const _<ISqlDriverRow>& previous) = 0;
};
