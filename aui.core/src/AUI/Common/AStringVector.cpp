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

#include "AStringVector.h"


AStringVector& AStringVector::noEmptyStrings()
{
	remove("");
	return *this;
}

AString AStringVector::join(AChar w) const
{
	AString res;

	for (const auto& x : *this)
	{
		if (!res.empty())
		{
			res += w;
		}
		res += x;
	}
	
	return res;
}

AString AStringVector::join(const AString& w) const {
    AString res;

    for (const auto& x : *this)
    {
        if (!res.empty())
        {
            res += w;
        }
        res += x;
    }

    return res;
}

