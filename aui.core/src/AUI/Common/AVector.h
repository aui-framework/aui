/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
	using p = std::vector<StoredType, Allocator>;
	using self = AVector<StoredType, Allocator>;

public:
    using p::p;
    using iterator = typename p::iterator;

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
        return insertAll(p::end(), c);
    }


    /**
     * Inserts all values of the specified container to the end.
     * @tparam OtherContainer other container type.
     * @param c other container
     * @return iterator pointing to the first element inserted.
     */
    template<typename OtherContainer>
    iterator insertAll(OtherContainer&& c) noexcept {
        return insertAll(p::end(), std::forward<OtherContainer>(c));
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
     * Inserts all values of the specified container.
     * @tparam OtherContainer other container type.
     * @param at position to insert at.
     * @param c other container
     * @return iterator pointing to the first element inserted.
     */
    template<typename OtherContainer>
    iterator insertAll(iterator at, OtherContainer&& c) noexcept {
        return p::insert(at, std::make_move_iterator(c.begin()), std::make_move_iterator(c.end()));
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

    [[nodiscard]]
    std::size_t sizeInBytes() const noexcept {
        return p::size() * sizeof(StoredType);
    }

    [[nodiscard]]
    StoredType& at(std::size_t index) {
        if (index >= p::size()) {
            aui::impl::outOfBoundsException();
        }
        return p::operator[](index);
    }

    [[nodiscard]]
    const StoredType& at(std::size_t index) const {
        if (index >= p::size()) {
            aui::impl::outOfBoundsException();
        }
        return p::operator[](index);
    }


    /**
     * Shortcut to <code>push_back</code>.
     * @param rhs value to push
     * @return self
     */
    self& operator<<(const StoredType& rhs) noexcept
    {
        p::push_back(rhs);
        return *this;
    }

    /**
     * Shortcut to <code>push_back</code>.
     * @param rhs value to push
     * @return self
     */
    self& operator<<(StoredType&& rhs) noexcept
    {
        p::push_back(std::forward<StoredType>(rhs));
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
        assert(("empty container could not have the first element" && !p::empty()));
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
        assert(("empty container could not have the first element" && !p::empty()));
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
        assert(("empty container could not have the last element" && !p::empty()));
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
        assert(("empty container could not have the last element" && !p::empty()));
        return p::back();
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
     * Removes element if predicate(container[i]) == true.
     * @param predicate predicate
     */
    template<typename Predicate>
    void removeIf(Predicate&& predicate) noexcept
    {
        p::erase(std::remove_if(p::begin(), p::end(), std::forward<Predicate>(predicate)), p::end());
    }

    template<typename Callable>
    inline static AVector<StoredType, Allocator> generate(size_t size, Callable&& callable) noexcept {
        AVector<StoredType, Allocator> s;
        s.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            s << callable(i);
        }
        return s;
    }

    ASet<StoredType> toSet() const noexcept {
        return ASet<StoredType>(p::begin(), p::end());
    }

    /**
     * @brief Constructs a new vector of transformed items of the range.
     * @param range items to transform from.
     * @param transformer transformer function.
     * @return A new vector.
     */
    template<typename Iterator, typename UnaryOperation>
    static auto fromRange(aui::range<Iterator> range, UnaryOperation&& transformer) -> AVector<decltype(transformer(range.first()))> {
        AVector<decltype(transformer(range.first()))> result;
        result.reserve(range.size());
        std::transform(range.begin(), range.end(), std::back_inserter(result), std::forward<UnaryOperation>(transformer));
        return result;
    }

    template<typename UnaryOperation>
    auto map(UnaryOperation&& transformer) const -> AVector<decltype(transformer(std::declval<StoredType>()))> {
        AVector<decltype(transformer(std::declval<StoredType>()))> result;
        result.reserve(p::size());
        std::transform(p::begin(), p::end(), std::back_inserter(result), std::forward<UnaryOperation>(transformer));
        return result;
    }

    template<typename UnaryOperation>
    [[nodiscard]]
    auto toMap(UnaryOperation&& transformer) const -> AMap<decltype(transformer(std::declval<StoredType>()).first),
                                                           decltype(transformer(std::declval<StoredType>()).second)> {
        return aui::container::to_map(p::begin(), p::end(), transformer);
    }

    template<typename UnaryOperation>
    [[nodiscard]]
    auto toMap(UnaryOperation&& transformer) -> AMap<decltype(transformer(std::declval<StoredType>()).first),
                                                     decltype(transformer(std::declval<StoredType>()).second)> {
        return aui::container::to_map(p::begin(), p::end(), transformer);
    }

    template<typename Predicate>
    AVector filter(Predicate&& predicate) {
        AVector result;
        result.reserve(p::size());
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

