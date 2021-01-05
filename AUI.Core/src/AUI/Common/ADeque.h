/**
 * =====================================================================================================================
 * Copyright (c) 2020 Alex2772
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
#include <deque>
#include "AUI/Core.h"
#include <algorithm>

template <class StoredType,
	class Allocator = std::allocator<StoredType>>
class ADeque: public std::deque<StoredType, Allocator>
{
	using parent = std::deque<StoredType, Allocator>;
public:
    ADeque() {}

    template<typename Iterator>
    ADeque(Iterator first, Iterator last) : parent(first, last) {}

    void remove(const StoredType& item)
	{
		parent::erase(std::remove_if(parent::begin(), parent::end(), [&](const StoredType& probe)
		{
			return item == probe;
		}), parent::end());
	}


	ADeque<StoredType, Allocator>& operator<<(const StoredType& rhs)
	{
		parent::push_back(rhs);
		return *this;
	}
	ADeque<StoredType, Allocator>& operator<<(StoredType&& rhs)
	{
		parent::push_back(std::forward<StoredType>(rhs));
		return *this;
	}

	bool contains(const StoredType& value) const
	{
		for (auto i = parent::begin(); i != parent::end(); ++i)
		{
			if (*i == value)
				return true;
		}
		return false;
	}

	template<typename Container>
	bool isSubsetOf(const Container& c) const
	{
		for (auto& i : c)
		{
			if (!contains(i))
			{
				return false;
			}
		}
		return true;
	}

	template<typename Func, typename... Args>
	void forEach(Func f, Args&& ... args)
	{
		for (auto& i : *this)
		{
			(i.get()->*f)(std::forward<Args>(args)...);
		}
	}

	StoredType& first()
	{
		return parent::front();
	}
	const StoredType& first() const
	{
		return parent::front();
	}

	StoredType& last()
	{
		return parent::back();
	}
	const StoredType& last() const
	{
		return parent::back();
	}
};


template <class StoredType, class Allocator = std::allocator<StoredType>>
ADeque<StoredType, Allocator>& operator<<(ADeque<StoredType, Allocator>& lhs, const StoredType& rhs)
{
	lhs.push_back(rhs);
	return lhs;
}