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
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once
#include <queue>
#include "AUI/Core.h"
#include <algorithm>

template <class StoredType>
	class AQueue : public std::queue<StoredType>
{
	using parent = std::queue<StoredType>;
public:
	void remove(const StoredType& item)
	{
		parent::erase(std::remove_if(parent::begin(), parent::end(), [&](const StoredType& probe)
		{
			return item == probe;
		}), parent::end());
	}


	AQueue<StoredType>& operator<<(const StoredType& rhs)
	{
		parent::push(rhs);
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
};


template <class StoredType>
AQueue<StoredType>& operator<<(AQueue<StoredType>& lhs, const StoredType& rhs)
{
	lhs.push(rhs);
	return lhs;
}