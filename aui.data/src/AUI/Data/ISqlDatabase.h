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
#include "AUI/Common/SharedPtrTypes.h"
#include "ISqlDriverResult.h"
#include "AUI/Common/AVariant.h"
#include "AUI/Common/AVector.h"
#include "AUI/Data/ASqlDriverType.h"

/*
 * @brief Driver-to-aui.data interface. See ASqlDatabase for Application-to-aui.data interface
 */
class ISqlDatabase
{
public:
	virtual ~ISqlDatabase() = default;
	virtual _<ISqlDriverResult> query(const AString& query, const AVector<AVariant>& params) = 0;
	virtual int execute(const AString& query, const AVector<AVariant>& params) = 0;

	virtual SqlDriverType getDriverType() = 0;
};
