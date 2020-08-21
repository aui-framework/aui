#pragma once

#include <AUI/Core.h>
#include <vector>
#include "SharedPtr.h"
#include <algorithm>

template <class StoredType, class Allocator = std::allocator<StoredType>>
class AVector: public std::vector<StoredType, Allocator>
{
protected:
	using parent = std::vector<StoredType, Allocator>;
	
public:

	inline AVector() = default;

	inline AVector(const Allocator& allocator)
		: parent(allocator)
	{
	}

	inline AVector(typename parent::size_type _Count, const Allocator& allocator = Allocator())
		: parent(_Count, allocator)
	{
	}

	inline AVector(typename parent::size_type _Count, const StoredType& _Val, const Allocator& allocator = Allocator())
		: parent(_Count, _Val, allocator)
	{
	}

	inline AVector(std::initializer_list<StoredType> _Ilist, const Allocator& allocator = Allocator())
		: parent(_Ilist, allocator)
	{
	}

	inline AVector(const parent& _Right)
		: parent(_Right)
	{
	}

	inline AVector(const parent& _Right, const Allocator& allocator = Allocator())
		: parent(_Right, allocator)
	{
	}

	inline AVector(parent&& _Right)
		: parent(_Right)
	{
	}

	inline AVector(parent&& _Right, const Allocator& allocator = Allocator())
		: parent(_Right, allocator)
	{
	}

	template <class Iterator>
	inline AVector(Iterator first, Iterator end, const Allocator& allocator = Allocator()): parent(first, end, allocator) {}

	inline void remove(const StoredType& item)
	{
		parent::erase(std::remove_if(parent::begin(), parent::end(), [&](const StoredType& probe)
		{
			return item == probe;
		}), parent::end());
	}


	inline AVector<StoredType, Allocator>& operator<<(const StoredType& rhs)
	{
		parent::push_back(rhs);
		return *this;
	}
	inline AVector<StoredType, Allocator>& operator<<(StoredType&& rhs)
	{
		parent::push_back(rhs);
		return *this;
	}
	inline AVector<StoredType, Allocator>& operator<<(const AVector<StoredType>& rhs)
	{
	    for (auto& item : rhs)
		    parent::push_back(item);
		return *this;
	}

	void removeAt(size_t index)
	{
		parent::erase(parent::begin() + index);
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
		for (auto i = parent::begin(); i != parent::end(); ++i, ++index)
		{
			if (*i == t)
				return index;
		}
		*/
		for (size_t i = 0; i < parent::size(); ++i)
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