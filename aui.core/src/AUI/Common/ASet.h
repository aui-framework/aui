﻿/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <set>
#include <ostream>
#include <AUI/Traits/containers.h>


/**
 * @brief A std::set with AUI extensions.
 * @ingroup core
 */
template <class KeyType, class Comparator = std::less<KeyType>, class Allocator = std::allocator<KeyType>>
class ASet: public std::set<KeyType, Comparator, Allocator>
{
private:
    using p = std::set<KeyType, Comparator, Allocator>;
    using self = ASet<KeyType, Comparator, Allocator>;

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
    void insertAll(const OtherContainer& c) noexcept {
        p::insert(c.begin(), c.end());
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
     * Shortcut to <code>push_back</code>.
     * @param rhs value to push
     * @return self
     */
    inline self& operator<<(const KeyType& rhs) noexcept
    {
        p::insert(rhs);
        return *this;
    }

    /**
     * Shortcut to <code>insert</code>.
     * @param rhs value to push
     * @return self
     */
    inline self& operator<<(KeyType&& rhs) noexcept
    {
        p::insert(std::move(rhs));
        return *this;
    }
    /**
     * Shortcut to <code>erase</code>.
     * @param rhs value to push
     * @return self
     */
    inline self& operator>>(const KeyType& rhs) noexcept
    {
        p::erase(rhs);
        return *this;
    }

    /**
     * Shortcut to <code>insertAll</code>.
     * @param rhs container to push
     * @return self
     */
    template<typename OtherContainer, std::enable_if_t<!std::is_convertible_v<OtherContainer, KeyType>, bool> = true>
    inline self& operator<<(const OtherContainer& c) noexcept
    {
        insertAll(c);
        return *this;
    }

    bool contains(const KeyType& value) const noexcept
    {
        return p::find(value) != p::end();
    }
};


template<typename T>
inline std::ostream& operator<<(std::ostream& o, const ASet<T>& v) {
    if (v.empty()) {
        o << "[empty]";
    } else {
        auto it = v.begin();
        o << "[ " << *it;
        ++it;
        for (; it != v.end(); ++it) {
            o << ", " << *it;
        }
        o << " ]";
    }

    return o;
}

