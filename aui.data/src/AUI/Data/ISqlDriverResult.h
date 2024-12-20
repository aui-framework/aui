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
