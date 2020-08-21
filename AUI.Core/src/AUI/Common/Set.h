#pragma once

#include "AUI/Core.h"
#include <set>


template <class KeyType, class Comparator = std::less<KeyType>, class Allocator = std::allocator<KeyType>>
class Set: public std::set<KeyType, Comparator, Allocator>
{
private:
	using parent = std::set<KeyType, Comparator, Allocator>;
	
public:

	template<typename Iterator>
	Set(Iterator begin, Iterator end): parent(begin, end)
	{
	}
	
	Set()
	{
	}

	Set(const typename parent::allocator_type& _Al)
		: parent(_Al)
	{
	}

	Set(const parent& _Right)
		: parent(_Right)
	{
	}

	Set(const parent& _Right, const typename parent::allocator_type& _Al)
		: parent(_Right, _Al)
	{
	}

	Set(const typename parent::key_compare& Comparatored)
		: parent(Comparatored)
	{
	}

	Set(const typename parent::key_compare& Comparatored, const typename parent::allocator_type& _Al)
		: parent(Comparatored, _Al)
	{
	}

	Set(parent&& _Right)
		: parent(_Right)
	{
	}

	Set(parent&& _Right, const typename parent::allocator_type& _Al)
		: parent(_Right, _Al)
	{
	}

	Set(std::initializer_list<KeyType> _Ilist)
		: parent(std::move(_Ilist))
	{
	}

	bool contains(const KeyType& value) const noexcept
	{
		return parent::find(value) != parent::end();
	}
	Set& operator<<(const KeyType& value) noexcept
	{
		parent::insert(value);
		return *this;
	}
};

namespace std
{
	template<typename T>
	struct hash<Set<T>>
	{
		size_t operator()(const Set<T>& t) const
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
