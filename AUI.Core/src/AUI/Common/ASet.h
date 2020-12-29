#pragma once

#include "AUI/Core.h"
#include <set>


template <class KeyType, class Comparator = std::less<KeyType>, class Allocator = std::allocator<KeyType>>
class ASet: public std::set<KeyType, Comparator, Allocator>
{
private:
	using parent = std::set<KeyType, Comparator, Allocator>;
	
public:

	template<typename Iterator>
	ASet(Iterator begin, Iterator end): parent(begin, end)
	{
	}
	
	ASet()
	{
	}

	ASet(const typename parent::allocator_type& _Al)
		: parent(_Al)
	{
	}

	ASet(const parent& _Right)
		: parent(_Right)
	{
	}

	ASet(const parent& _Right, const typename parent::allocator_type& _Al)
		: parent(_Right, _Al)
	{
	}

	ASet(const typename parent::key_compare& Comparatored)
		: parent(Comparatored)
	{
	}

	ASet(const typename parent::key_compare& Comparatored, const typename parent::allocator_type& _Al)
		: parent(Comparatored, _Al)
	{
	}

	ASet(parent&& _Right)
		: parent(_Right)
	{
	}

	ASet(parent&& _Right, const typename parent::allocator_type& _Al)
		: parent(_Right, _Al)
	{
	}

	ASet(std::initializer_list<KeyType> _Ilist)
		: parent(std::move(_Ilist))
	{
	}

	bool contains(const KeyType& value) const noexcept
	{
		return parent::find(value) != parent::end();
	}
	ASet& operator<<(const KeyType& value) noexcept
	{
		parent::insert(value);
		return *this;
	}
};

namespace std
{
	template<typename T>
	struct hash<ASet<T>>
	{
		size_t operator()(const ASet<T>& t) const
		{
			size_t totalHash = 1298511242;
			for (auto i = t.begin(); i != t.end(); ++i)
			{
				totalHash ^= hash<T>()(*i);
			}
			return totalHash;
		}
	};
}
