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

#include <AUI/Core.h>
#include <vector>
#include <cassert>
#include "SharedPtrTypes.h"
#include <algorithm>
#include <ostream>
#include "ASet.h"
#include <AUI/Traits/containers.h>
#include <AUI/Traits/iterators.h>
#include "AContainerPrototypes.h"
#include "AUI/Traits/concepts.h"
#include <concepts>


namespace aui::impl {
    API_AUI_CORE void outOfBoundsException();
}

/**
 * @brief A std::vector with AUI extensions.
 * @ingroup core
 */
template <class StoredType, class Allocator>
class AVector: public std::vector<StoredType, Allocator>
{
protected:
	using super = std::vector<StoredType, Allocator>;
	using self = AVector<StoredType, Allocator>;

public:
    using super::super;
    using iterator = typename super::iterator;

    template<typename Iterator>
    explicit AVector(aui::range<Iterator> range): AVector(range.begin(), range.end()) {}


    /**
     * Inserts all values of the specified container to the end.
     * @tparam OtherContainer other container type.
     * @param c other container
     * @return iterator pointing to the first element inserted.
     */
    template<typename OtherContainer>
    iterator insertAll(const OtherContainer& c) noexcept {
        return insertAll(super::end(), c);
    }


    /**
     * Inserts all values of the specified container to the end.
     * @tparam OtherContainer other container type.
     * @param c other container
     * @return iterator pointing to the first element inserted.
     */
    template<typename OtherContainer>
    iterator insertAll(OtherContainer&& c) noexcept {
        return insertAll(super::end(), std::forward<OtherContainer>(c));
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
        return super::insert(at, c.begin(), c.end());
    }


    /**
     * Inserts all values of the specified container.
     * @tparam OtherContainer other container type.
     * @param at position to insert at.
     * @param c other container
     * @return iterator pointing to the first element inserted.
     */
    template<typename OtherContainer>
    iterator insertAll(iterator at, OtherContainer&& c) noexcept {
        return super::insert(at, std::make_move_iterator(c.begin()), std::make_move_iterator(c.end()));
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
     * @return If the item is removed, it's index returned.
     */
    AOptional<std::size_t> removeFirst(const StoredType& item) noexcept
    {
        return aui::container::remove_first(*this, item);
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

    [[nodiscard]]
    std::size_t sizeInBytes() const noexcept {
        return super::size() * sizeof(StoredType);
    }

    [[nodiscard]]
    StoredType& at(std::size_t index) {
        if (index >= super::size()) {
            aui::impl::outOfBoundsException();
        }
        return super::operator[](index);
    }

    [[nodiscard]]
    const StoredType& at(std::size_t index) const {
        if (index >= super::size()) {
            aui::impl::outOfBoundsException();
        }
        return super::operator[](index);
    }


    /**
     * Shortcut to <code>push_back</code>.
     * @param rhs value to push
     * @return self
     */
    self& operator<<(const StoredType& rhs) noexcept
    {
        super::push_back(rhs);
        return *this;
    }

    /**
     * Shortcut to <code>push_back</code>.
     * @param rhs value to push
     * @return self
     */
    self& operator<<(StoredType&& rhs) noexcept
    {
        super::push_back(std::forward<StoredType>(rhs));
        return *this;
    }

    /**
     * Shortcut to <code>insertAll</code>.
     * @param rhs container to push
     * @return self
     */
    template<typename OtherContainer, std::enable_if_t<!std::is_convertible_v<OtherContainer, StoredType>, bool> = true>
    self& operator<<(const OtherContainer& c) noexcept
    {
        insertAll(c);
        return *this;
    }

    /**
     * Shortcut to <code>insertAll</code>.
     * @param rhs container to push
     * @return self
     */
    template<typename OtherContainer, std::enable_if_t<!std::is_convertible_v<OtherContainer, StoredType>, bool> = true>
    self& operator<<(OtherContainer&& c) noexcept
    {
        insertAll(std::forward<OtherContainer>(c));
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
        assert(("empty container could not have the first element" && !super::empty()));
        return super::front();
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
        assert(("empty container could not have the first element" && !super::empty()));
        return super::front();
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
        assert(("empty container could not have the last element" && !super::empty()));
        return super::back();
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
        assert(("empty container could not have the last element" && !super::empty()));
        return super::back();
    }

    /**
     * @param value element to find.
     * @return index of the specified element. If element is not found, -1 is returned.
     */
    [[nodiscard]]
    size_t indexOf(const StoredType& value) const noexcept
    {
        return aui::container::index_of(*this, value);
    }


    AVector<StoredType>& sort() noexcept {
        std::sort(super::begin(), super::end());
        return *this;
    }

    template<typename Comparator>
    AVector<StoredType>& sort(Comparator&& comparator) noexcept {
        std::sort(super::begin(), super::end(), std::forward<Comparator>(comparator));
        return *this;
    }

    /**
     * @brief Finds element by predicate
     * @param predicate predicate
     * @return Pointer to the value on which the predicate returned true, nullptr otherwise
     */
    template<aui::predicate<StoredType> Predicate>
    StoredType* findIf(Predicate&& predicate) noexcept
    {
        if (auto i = std::find_if(super::begin(), super::end(), std::forward<Predicate>(predicate)); i != super::end()) {
            return &*i;
        }
        return nullptr;
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
     * Removes element if predicate(container[i]) == true.
     * @param predicate predicate
     */
    template<aui::predicate<StoredType> Predicate>
    void removeIf(Predicate&& predicate) noexcept
    {
        super::erase(std::remove_if(super::begin(), super::end(), std::forward<Predicate>(predicate)), super::end());
    }

    /**
     * Removes only the first element if predicate(container[i]) == true.
     * @param predicate predicate
     */
    template<aui::predicate<StoredType> Predicate>
    void removeIfFirst(Predicate&& predicate) noexcept
    {
        auto i = std::find_if(super::begin(), super::end(), std::forward<Predicate>(predicate));
        if (i == super::end()) {
            return;
        }
        super::erase(i);
    }

    template<aui::mapper<std::size_t, StoredType> Callable>
    inline static AVector<StoredType, Allocator> generate(size_t size, Callable&& callable) noexcept {
        AVector<StoredType, Allocator> s;
        s.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            s << callable(i);
        }
        return s;
    }

    ASet<StoredType> toSet() const noexcept {
        return ASet<StoredType>(super::begin(), super::end());
    }

    /**
     * @brief Constructs a new vector of transformed items of the range.
     * @param range items to transform from.
     * @param transformer transformer function.
     * @return A new vector.
     */
    template<aui::incrementable Iterator, aui::invocable<decltype(*std::declval<Iterator>())> UnaryOperation>
    static auto fromRange(aui::range<Iterator> range, UnaryOperation&& transformer) -> AVector<decltype(transformer(range.first()))> {
        AVector<decltype(transformer(range.first()))> result;
        result.reserve(range.size());
        std::transform(range.begin(), range.end(), std::back_inserter(result), std::forward<UnaryOperation>(transformer));
        return result;
    }

    template<aui::invocable<const StoredType&> UnaryOperation>
    auto map(UnaryOperation&& transformer) const -> AVector<decltype(transformer(std::declval<StoredType>()))> {
        AVector<decltype(transformer(std::declval<StoredType>()))> result;
        result.reserve(super::size());
        std::transform(super::begin(), super::end(), std::back_inserter(result), std::forward<UnaryOperation>(transformer));
        return result;
    }

    template<aui::invocable<const StoredType&> UnaryOperation>
    [[nodiscard]]
    auto toMap(UnaryOperation&& transformer) const -> AMap<decltype(transformer(std::declval<StoredType>()).first),
                                                           decltype(transformer(std::declval<StoredType>()).second)> {
        return aui::container::to_map(super::begin(), super::end(), transformer);
    }

    template<aui::invocable<StoredType&> UnaryOperation>
    [[nodiscard]]
    auto toMap(UnaryOperation&& transformer) -> AMap<decltype(transformer(std::declval<StoredType>()).first),
                                                     decltype(transformer(std::declval<StoredType>()).second)> {
        return aui::container::to_map(super::begin(), super::end(), transformer);
    }

    template<aui::predicate<const StoredType&> Predicate>
    self filter(Predicate&& predicate) {
        self result;
        result.reserve(super::size());
        for (const auto& element : *this) {
            if (predicate(element)) {
                result.push_back(element);
            }
        }
        return result;
    }
};


template<typename T>
inline std::ostream& operator<<(std::ostream& o, const AVector<T>& v) {
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

