/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <map>
#include <unordered_map>
#include "AUI/Core.h"
#include "AException.h"
#include <AUI/Common/AVector.h>
#include <AUI/Traits/containers.h>
#include <AUI/Reflect/AClass.h>
#include "AContainerPrototypes.h"

/**
 * @brief Base class for maps with AUI extensions.
 * @ingroup core
 */
template <class KeyType, class ValueType, class Parent>
class ABaseMap: public Parent
{
public:
	using iterator = typename Parent::iterator;
	using const_iterator = typename Parent::const_iterator;

	using Parent::Parent;

	ValueType& operator[](KeyType&& k)
	{
		return Parent::operator[](std::move(k));
	}

	ValueType& operator[](const KeyType& k)
	{
		return Parent::operator[](k);
	}

	const ValueType& operator[](KeyType&& k) const
	{
		return Parent::at(std::move(k));
	}

	const ValueType& operator[](const KeyType& k) const
	{
		return Parent::at(k);
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
	    auto it = Parent::find(key);
	    if (it == Parent::end())
	        throw AException("no such element: {}"_format(key));
	    return it->second;
	}
	const ValueType& at(const KeyType& key) const {
	    auto it = Parent::find(key);
	    if (it == Parent::end())
	        throw AException("no such element: {}"_format(key));
	    return it->second;
	}

    [[nodiscard]]
	const_contains_iterator contains(const KeyType& key) const noexcept
	{
		auto it = Parent::find(key);
		return const_contains_iterator(it, it != Parent::end());
	}

    [[nodiscard]]
    contains_iterator contains(const KeyType& key) noexcept
	{
		auto it = Parent::find(key);
		return contains_iterator(it, it != Parent::end());
	}

    [[nodiscard]]
    AOptional<ValueType> optional(const KeyType& key) const noexcept {
        auto it = Parent::find(key);
        if (it == Parent::end()) {
            return std::nullopt;
        }
        return it->second;
    }

	AVector<KeyType> keyVector() {
        AVector<KeyType> r;
        r.reserve(Parent::size());
        for (auto& p : *this) {
            r << p.first;
        }
        return r;
	}
	AVector<ValueType> valueVector() {
        AVector<ValueType> r;
        r.reserve(Parent::size());
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
        auto[it, isElementCreated] = Parent::insert(typename Parent::value_type(keyType, ValueType{}));
        static_assert(std::is_same_v<decltype(it), typename Parent::iterator>, "govno");
        if (isElementCreated) {
            it->second = factory();
        }
        return it->second;
    }

    template<typename BinaryOperation>
    auto toVector(BinaryOperation&& transformer) const -> AVector<decltype(transformer(std::declval<KeyType>(), std::declval<ValueType>()))> {
        AVector<decltype(transformer(std::declval<KeyType>(), std::declval<ValueType>()))> result;
        result.reserve(Parent::size());
        std::transform(Parent::begin(), Parent::end(), std::back_inserter(result), [transformer = std::forward<BinaryOperation>(transformer)](const typename Parent::value_type& p){
            return transformer(p.first, p.second);
        });
        return result;
    }

    AVector<std::tuple<KeyType, ValueType>> toVector() const {
        return toVector([](const KeyType& k, const ValueType& v) {
            return std::make_tuple(k, v);
        });
    }
};

/**
 * @brief A std::map with AUI extensions.
 * @ingroup core
 */
template <class KeyType, class ValueType, class Predicate, class Allocator>
class AMap: public ABaseMap<KeyType, ValueType, std::map<KeyType, ValueType, Predicate, Allocator>>
{
	using parent = ABaseMap<KeyType, ValueType, std::map<KeyType, ValueType, Predicate, Allocator>>;

	using parent::parent;
};

/**
 * @brief A std::unordered_map with AUI extensions.
 * @ingroup core
 */
template <class KeyType, class ValueType, class Hasher, class Comparer, class Allocator>
class AUnorderedMap: public ABaseMap<KeyType, ValueType, std::unordered_map<KeyType, ValueType, Hasher, Comparer, Allocator>>
{
	using parent = ABaseMap<KeyType, ValueType, std::unordered_map<KeyType, ValueType, Hasher, Comparer, Allocator>>;

	using parent::parent;
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

template<typename Iterator, typename UnaryOperation>
inline auto aui::container::to_unordered_map(Iterator begin,
                                             Iterator end,
                                             UnaryOperation&& transformer) {
    AUnorderedMap<decltype(transformer(*begin).first),
                  decltype(transformer(*begin).second)> result;

    for (auto it = begin; it != end; ++it) {
        auto[key, value] = transformer(*it);
        result[std::move(key)] = std::move(value);
    }
    return result;
}