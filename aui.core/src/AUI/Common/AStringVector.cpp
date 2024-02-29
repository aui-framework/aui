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

#include "AStringVector.h"


AStringVector& AStringVector::noEmptyStrings()
{
	remove("");
	return *this;
}

AString AStringVector::join(wchar_t w) const
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

