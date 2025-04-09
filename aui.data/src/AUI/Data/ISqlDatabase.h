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
