#pragma once
#include <queue>
#include "AUI/Core.h"
#include <algorithm>

template <class StoredType>
	class Queue : public std::queue<StoredType>
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


	Queue<StoredType>& operator<<(const StoredType& rhs)
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
Queue<StoredType>& operator<<(Queue<StoredType>& lhs, const StoredType& rhs)
{
	lhs.push(rhs);
	return lhs;
}