/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
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
#include "AString.h"
#include "AVector.h"

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

	AStringVector(p::size_type _Count, const std::allocator<AString>& allocator)
		: AVector<AString>(_Count, allocator)
	{
	}

	AStringVector(p::size_type _Count, const AString& _Val, const std::allocator<AString>& allocator)
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
