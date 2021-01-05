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

#include <AUI/Core.h>
#include <vector>
#include "SharedPtr.h"
#include <algorithm>

template <class StoredType, class Allocator = std::allocator<StoredType>>
class AVector: public std::vector<StoredType, Allocator>
{
protected:
	using p = std::vector<StoredType, Allocator>;
	
public:

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

	inline void remove(const StoredType& item)
	{
		this->erase(std::remove_if(this->begin(), this->end(), [&](const StoredType& probe)
		{
			return item == probe;
		}), this->end());
	}

    void push_back(const StoredType& data) {
        p::push_back(data);
    }

    void push_back(StoredType&& data) {
        p::push_back(data);
    }


	inline AVector<StoredType, Allocator>& operator<<(const StoredType& rhs)
	{
		p::push_back(rhs);
		return *this;
	}
	inline AVector<StoredType, Allocator>& operator<<(StoredType&& rhs)
	{
		p::push_back(std::forward<StoredType>(rhs));
		return *this;
	}
	inline AVector<StoredType, Allocator>& operator<<(const AVector<StoredType>& rhs)
	{
	    for (auto& item : rhs)
		    p::push_back(item);
		return *this;
	}

	void removeAt(size_t index)
	{
		this->erase(this->begin() + index);
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

    template<typename Container>
	Iterator insertAll(Iterator position, const Container& c) {
	    return p::insert(position, c.begin(), c.end());
	}

	[[nodiscard]]
	size_t indexOf(const StoredType& t) const
	{
		/*
		 * leave this for other container implementations
		 * 
		size_t index = 0;
		for (auto i = this->begin(); i != this->end(); ++i, ++index)
		{
			if (*i == t)
				return index;
		}
		*/
		for (size_t i = 0; i < this->size(); ++i)
		{
			if ((*this)[i] == t)
				return i;
		}
		
		return static_cast<size_t>(-1);
	}

	template<typename Func, typename... Args>
	void forEach(Func f, Args&&... args)
	{
		for (auto& i: *this)
		{
			(i.get()->*f)(std::forward<Args>(args)...);
		}
	}

	StoredType& first()
	{
		return this->front();
	}
	const StoredType& first() const
	{
		return this->front();
	}

	StoredType& last()
	{
		return this->back();
	}
	const StoredType& last() const
	{
		return this->back();
	}

	bool contains(const StoredType& value) const {
	    return std::find(p::begin(), p::end(), value) != p::end();
	}
};