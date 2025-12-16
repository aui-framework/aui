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

#include <deque>
#include <algorithm>
#include <AUI/Core.h>
#include <AUI/Common/ASet.h>
#include <AUI/Traits/containers.h>


/**
 * @brief A std::deque with AUI extensions.
 * @ingroup core
 */
template <class StoredType,
	class Allocator = std::allocator<StoredType>>
class ADeque: public std::deque<StoredType, Allocator>
{
private:
    using p = std::deque<StoredType, Allocator>;
    using self = ADeque<StoredType, Allocator>;
public:
    using p::p;
    using iterator = typename p::iterator;


    /**
     * Inserts all values of the specified container to the end.
     * @tparam OtherContainer other container type.
     * @param c other container
     * @return iterator pointing to the first element inserted.
     */
    template<typename OtherContainer>
    iterator insertAll(const OtherContainer& c) noexcept {
        return insertAll(p::end(), c);
    }


    /**
     * Inserts all values of the specified container.
     * @tparam OtherContainer other container type.
     * @param at position to insert at.
     * @param c other container
     * @return iterator pointing to the first element inserted.
     */
    template<typename OtherContainer>
    iterator insertAll(iterator at, const OtherContainer& c) noexcept {
        return p::insert(at, c.begin(), c.end());
    }


    /**
     * Removes all occurrences of <code>item</code>.
     * @param item element to remove.
     */
    void removeAll(const StoredType& item) noexcept
    {
        aui::container::remove_all(*this, item);
    }

    /**
     * Removes first occurrence of <code>item</code>.
     * @param item element to remove.
     */
    void removeFirst(const StoredType& item) noexcept
    {
        aui::container::remove_first(*this, item);
    }

    /**
     * @return true if <code>c</code> container is a subset of this container, false otherwise.
     */
    template<typename OtherContainer>
    bool isSubsetOf(const OtherContainer& c) const noexcept
    {
        return aui::container::is_subset(*this, c);
    }

    /**
     * @return true if container contains an element, false otherwise.
     */
    bool contains(const StoredType& value) const noexcept {
        return aui::container::contains(*this, value);
    }


    /**
     * Shortcut to <code>push_back</code>.
     * @param rhs value to push
     * @return self
     */
    inline self& operator<<(const StoredType& rhs) noexcept
    {
        p::push_back(rhs);
        return *this;
    }

    /**
     * Shortcut to <code>push_back</code>.
     * @param rhs value to push
     * @return self
     */
    inline self& operator<<(StoredType&& rhs) noexcept
    {
        p::push_back(std::move(rhs));
        return *this;
    }

    /**
     * Shortcut to <code>insertAll</code>.
     * @param c container to push
     * @return self
     */
    template<typename OtherContainer>
    inline self& operator<<(const OtherContainer& c) noexcept
    requires (!std::is_convertible_v<OtherContainer, StoredType>) {
        insertAll(c);
        return *this;
    }


    /**
     * <dl>
     *   <dt><b>Sneaky assertions</b></dt>
     *   <dd>Container is not empty.</dd>
     * </dl>
     * @return the first element.
     */
    StoredType& first() noexcept
    {
        AUI_ASSERTX(!p::empty(), "empty container could not have the first element");
        return p::front();
    }

    /**
     * <dl>
     *   <dt><b>Sneaky assertions</b></dt>
     *   <dd>Container is not empty.</dd>
     * </dl>
     * @return the first element.
     */
    const StoredType& first() const noexcept
    {
        AUI_ASSERTX(!p::empty(), "empty container could not have the first element");
        return p::front();
    }

    /**
     * <dl>
     *   <dt><b>Sneaky assertions</b></dt>
     *   <dd>Container is not empty.</dd>
     * </dl>
     * @return the last element.
     */
    StoredType& last() noexcept
    {
        AUI_ASSERTX(!p::empty(), "empty container could not have the last element");
        return p::back();
    }

    /**
     * <dl>
     *   <dt><b>Sneaky assertions</b></dt>
     *   <dd>Container is not empty.</dd>
     * </dl>
     * @return the last element.
     */
    const StoredType& last() const noexcept
    {
        AUI_ASSERTX(!p::empty(), "empty container could not have the last element");
        return p::back();
    }

    /**
     * @param value element to find.
     * @return index of the specified element. If element is not found, std::nullopt is returned.
     */
    [[nodiscard]]
    AOptional<size_t> indexOf(const StoredType& value) const noexcept
    {
        return aui::container::index_of(*this, value);
    }


    void sort() noexcept {
        std::sort(p::begin(), p::end());
    }

    template<typename Comparator>
    void sort(Comparator&& comparator) noexcept {
        std::sort(p::begin(), p::end(), std::forward<Comparator>(comparator));
    }

    /**
     * Removes element at the specified index.
     * <dl>
     *   <dt><b>Sneaky assertions</b></dt>
     *   <dd><code>index</code> points to the existing element.</dd>
     * </dl>
     * @param index index of the element.
     */
    void removeAt(size_t index) noexcept
    {
        aui::container::remove_at(*this, index);
    }

    /**
     * Removes element if `predicate(container[i]) == true`.
     * @param predicate predicate
     */
    template<typename Predicate>
    void removeIf(Predicate&& predicate) noexcept
    {
        p::erase(std::remove_if(p::begin(), p::end(), std::forward<Predicate>(predicate)), p::end());
    }


    ASet<StoredType> toSet() const noexcept {
        return ASet<StoredType>(p::begin(), p::end());
    }
};

template<typename T>
inline std::ostream& operator<<(std::ostream& o, const ADeque<T>& v) {
    if (v.empty()) {
        o << "[empty]";
    } else {
        o << "[ " << v.first();
        for (auto it = v.begin() + 1; it != v.end(); ++it) {
            o << ", " << *it;
        }
        o << " ]";
    }

    return o;
}

