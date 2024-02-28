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

#pragma once
#include "AString.h"
#include "AVector.h"


/**
 * @brief An AVector with string-related functions.
 * @ingroup core
 */
class API_AUI_CORE AStringVector: public AVector<AString>
{
public:
	AStringVector()
	{
	}
	AStringVector(const AVector<AString>& v) : AVector<AString>(v)
	{
	}

    template<typename Iterator>
	AStringVector(Iterator begin, Iterator end) : AVector<AString>(begin, end)
	{
	}

	AStringVector(const std::allocator<AString>& allocator)
		: AVector<AString>(allocator)
	{
	}

	AStringVector(super::size_type _Count, const std::allocator<AString>& allocator)
		: AVector<AString>(_Count, allocator)
	{
	}

	AStringVector(super::size_type _Count, const AString& _Val, const std::allocator<AString>& allocator)
		: AVector<AString>(_Count, _Val, allocator)
	{
	}

	AStringVector(std::initializer_list<AString> _Ilist, const std::allocator<AString>& allocator = std::allocator<AString>())
		: AVector<AString>(std::move(_Ilist), allocator)
	{
	}

	AStringVector& noEmptyStrings();

	[[nodiscard]] AString join(wchar_t w) const;
	[[nodiscard]] AString join(const AString& w) const;

};
