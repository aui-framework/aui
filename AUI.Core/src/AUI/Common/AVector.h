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


	inline AVector<StoredType, Allocator>& operator<<(const StoredType& rhs)
	{
		this->push_back(rhs);
		return *this;
	}
	inline AVector<StoredType, Allocator>& operator<<(StoredType&& rhs)
	{
		this->push_back(rhs);
		return *this;
	}
	inline AVector<StoredType, Allocator>& operator<<(const AVector<StoredType>& rhs)
	{
	    for (auto& item : rhs)
		    this->push_back(item);
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

	[[nodiscard]]
	size_t indexOf(const StoredType& t) const
	{
		/*
		 * Это оставим для других контейнеров
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
};