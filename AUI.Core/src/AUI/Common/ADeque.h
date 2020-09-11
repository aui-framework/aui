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