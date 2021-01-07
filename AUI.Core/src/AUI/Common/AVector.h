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

#include <AUI/Core.h>
#include <vector>
#include "SharedPtr.h"
#include <algorithm>
#include <AUI/Util/Extensions/SequenceContainerExtensions.h>

template <class StoredType, class Allocator = std::allocator<StoredType>>
class AVector: public SequenceContainerExtensions<std::vector<StoredType, Allocator>>
{
protected:
	using p = SequenceContainerExtensions<std::vector<StoredType, Allocator>>;
	
public:

    using ElementType = typename p::value_type;
    using Iterator = typename p::iterator;
    using ConstIterator = typename p::const_iterator;

	inline AVector() = default;

	inline AVector(const Allocator& allocator)
		: p(allocator)
	{
	}

	inline AVector(typename p::size_type _Count, const Allocator& allocator = Allocator())
		: p(_Count, allocator)
	{
	}

	inline AVector(typename p::size_type _Count, const StoredType& _Val, const Allocator& allocator = Allocator())
		: p(_Count, _Val, allocator)
	{
	}

	inline AVector(std::initializer_list<StoredType> _Ilist, const Allocator& allocator = Allocator())
		: p(_Ilist, allocator)
	{
	}

	inline AVector(const p& _Right)
		: p(_Right)
	{
	}

	inline AVector(const p& _Right, const Allocator& allocator = Allocator())
		: p(_Right, allocator)
	{
	}

	inline AVector(p&& _Right)
		: p(_Right)
	{
	}

	inline AVector(p&& _Right, const Allocator& allocator = Allocator())
		: p(_Right, allocator)
	{
	}

	template <class Iterator>
	inline AVector(Iterator first, Iterator end, const Allocator& allocator = Allocator()): p(first, end, allocator) {}

};