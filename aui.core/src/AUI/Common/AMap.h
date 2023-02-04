// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <map>
#include "AUI/Core.h"
#include "AException.h"
#include <AUI/Common/AVector.h>
#include <AUI/Traits/containers.h>
#include <AUI/Reflect/AClass.h>
#include "AContainerPrototypes.h"

/**
 * @brief A std::map with AUI extensions.
 * @ingroup core
 */
template <class KeyType, class ValueType, class Predicate, class Allocator>
class AMap: public std::map<KeyType, ValueType, Predicate, Allocator>
{
public:
	using parent = std::map<KeyType, ValueType, Predicate, Allocator>;
	using iterator = typename parent::iterator;
	using const_iterator = typename parent::const_iterator;

	using parent::parent;

	ValueType& operator[](KeyType&& k)
	{
		return parent::operator[](std::forward<KeyType>(k));
	}

	ValueType& operator[](const KeyType& k)
	{
		return parent::operator[](k);
	}

	const ValueType& operator[](KeyType&& k) const
	{
		return parent::at(std::forward<KeyType>(k));
	}

	const ValueType& operator[](const KeyType& k) const
	{
		return parent::at(k);
	}

	// ================

	struct contains_iterator
	{
	private:
		iterator mIterator;
		bool mValid;
		
	public:
		contains_iterator(const iterator& p, bool valid):
			mIterator(p),
			mValid(valid)
		{
			
		}
        contains_iterator(const contains_iterator& c):
			mIterator(c.mIterator),
			mValid(c.mValid)
		{
			
		}

		operator bool() const noexcept {
			return mValid;
		}

		iterator operator->() const noexcept
		{
			return mIterator;
		}
		iterator operator*() const noexcept
		{
			return mIterator;
		}
	};
	struct const_contains_iterator
	{
	private:
		const_iterator mIterator;
		bool mValid;

	public:
		const_contains_iterator(const const_iterator& p, bool valid):
			mIterator(p),
			mValid(valid)
		{

		}
        const_contains_iterator(const const_contains_iterator& c):
			mIterator(c.mIterator),
			mValid(c.mValid)
		{

		}

		operator bool() const noexcept {
			return mValid;
		}

		const_iterator operator->() const noexcept
		{
			return mIterator;
		}
		const_iterator operator*() const noexcept
		{
			return mIterator;
		}
	};

	ValueType& at(const KeyType& key) {
	    auto it = parent::find(key);
	    if (it == parent::end())
	        throw AException("no such element: " + AClass<KeyType>::toString(key));
	    return it->second;
	}
	const ValueType& at(const KeyType& key) const {
	    auto it = parent::find(key);
	    if (it == parent::end())
	        throw AException("no such element: " + AClass<KeyType>::toString(key));
	    return it->second;
	}

	const_contains_iterator contains(const KeyType& key) const
	{
		auto it = parent::find(key);
		return const_contains_iterator(it, it != parent::end());
	}

    contains_iterator contains(const KeyType& key)
	{
		auto it = parent::find(key);
		return contains_iterator(it, it != parent::end());
	}

	AVector<KeyType> keyVector() {
        AVector<KeyType> r;
        r.reserve(parent::size());
        for (auto& p : *this) {
            r << p.first;
        }
        return r;
	}
	AVector<ValueType> valueVector() {
        AVector<ValueType> r;
        r.reserve(parent::size());
        for (auto& p : *this) {
            r << p.second;
        }
        return r;
	}

    /**
     * If value of specified key exists, the existing value returned. Otherwise, <code>factory</code> function called.
     * @param keyType key;
     * @param factory factory function that will return a new value. Called only if map does not contain value of
     *        specified key.
     * @return stored or new value.
     */
    template<typename Factory>
    ValueType& getOrInsert(const KeyType& keyType, Factory&& factory) noexcept(noexcept(factory())) {
        static_assert(std::is_constructible_v<ValueType>, "ValueType is expected to be default-constructible");
        auto[it, isElementCreated] = parent::insert(typename parent::value_type(keyType, ValueType{}));
        static_assert(std::is_same_v<decltype(it), typename parent::iterator>, "govno");
        if (isElementCreated) {
            it->second = factory();
        }
        return it->second;
    }

    template<typename BinaryOperation>
    auto toVector(BinaryOperation&& transformer) const -> AVector<decltype(transformer(std::declval<KeyType>(), std::declval<ValueType>()))> {
        AVector<decltype(transformer(std::declval<KeyType>(), std::declval<ValueType>()))> result;
        result.reserve(parent::size());
        std::transform(parent::begin(), parent::end(), std::back_inserter(result), [transformer = std::forward<BinaryOperation>(transformer)](const typename parent::value_type& p){
            return transformer(p.first, p.second);
        });
        return result;
    }
};


template<typename Iterator, typename UnaryOperation>
inline auto aui::container::to_map(Iterator begin,
                                   Iterator end,
                                   UnaryOperation&& transformer) {
    AMap<decltype(transformer(*begin).first),
         decltype(transformer(*begin).second)> result;

    for (auto it = begin; it != end; ++it) {
        auto[key, value] = transformer(*it);
        result[std::move(key)] = std::move(value);
    }
    return result;
}