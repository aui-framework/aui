//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <cstddef>

#include "ADynamicVector.h"
#include "AStaticVector.h"

/**
 * @brief Vector-like container consisting of few elements on stack and switches to dynamic allocation vector if needed.
 * @tparam StoredType stored type
 * @tparam StaticVectorSize how many elements can be stored without dynamic allocation
 * @tparam Allocator allocation
 * @ingroup core
 *
 * @details
 * Vector-like container optimized for the case when it contains up to StaticVectorSize in place, avoiding dynamic
 * allocation. In case when element count exceeds StaticVectorSize, ASmallVector seamlessly switches to traditional
 * dynamic allocation vector.
 */
template<typename StoredType, std::size_t StaticVectorSize, typename Allocator = std::allocator<StoredType>>
class ASmallVector {
protected:
    using self = ASmallVector;
    using super = self;
    using StaticVector = AStaticVector<StoredType, StaticVectorSize>;
    using DynamicVector = ADynamicVector<StoredType>;

public:
    using iterator = StoredType*;
    using const_iterator = const StoredType*;
    using reference = StoredType&;
    using const_reference = const StoredType&;
    using value = StoredType;

    ASmallVector() noexcept {
        new (&mBase.inplace) StaticVector();
    }

    ASmallVector(ASmallVector&& rhs) noexcept {
        if (rhs.isInplaceAllocated()) {
            new (&mBase.inplace) StaticVector(*rhs.inplace());
        } else {
            new (&mBase.dynamic) DynamicVector(*rhs.dynamic());
        }

        new (&rhs.mBase.inplace) StaticVector();
    }

    ~ASmallVector() noexcept {
        deallocate();
    }

    [[nodiscard]]
    constexpr StoredType* data() noexcept {
        return reinterpret_cast<StoredType*>(mBase.common.begin);
    }
    [[nodiscard]]
    constexpr const StoredType* data() const noexcept {
        return reinterpret_cast<const StoredType*>(mBase.common.begin);
    }

    [[nodiscard]]
    constexpr iterator begin() noexcept {
        return mBase.common.begin;
    }
    [[nodiscard]]
    constexpr const_iterator begin() const noexcept {
        return mBase.common.begin;
    }

    [[nodiscard]]
    constexpr iterator end() noexcept {
        return mBase.common.end;
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept {
        return mBase.common.end;
    }

    [[nodiscard]]
    constexpr StoredType& front() noexcept {
        return *begin();
    }

    [[nodiscard]]
    constexpr StoredType& back() noexcept {
        return *(begin() + (size() - 1));
    }

    [[nodiscard]]
    constexpr const StoredType& front() const noexcept {
        return *begin();
    }

    [[nodiscard]]
    constexpr const StoredType& back() const noexcept {
        return *(begin() + (size() - 1));
    }

    constexpr void push_back(StoredType value) noexcept {
        insert(end(), std::move(value));
    }

    constexpr void push_front(StoredType value) noexcept {
        insert(begin(), std::move(value));
    }

    constexpr void pop_back() noexcept {
        assert(("ASmallVector is empty", size() > 0));
        erase(std::prev(end()));
    }
    constexpr void pop_front() noexcept {
        assert(("ASmallVector is empty", size() > 0));
        erase(begin());
    }

    [[nodiscard]]
    constexpr StoredType& operator[](std::size_t index) noexcept {
        assert(("out of bounds", index < size()));
        return *(data() + index);
    }

    [[nodiscard]]
    constexpr StoredType& operator[](std::size_t index) const noexcept {
        return const_cast<ASmallVector*>(this)->operator[](index);
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept {
        return begin() == end();
    }

    constexpr void clear() noexcept {
        deallocate();
        new (inplace()) StaticVector();
    }

    /**
     * @return Size of the container.
     */
    [[nodiscard]]
    std::size_t size() const noexcept {
        return std::distance(begin(), end());
    }


    [[nodiscard]]
    bool isInplaceAllocated() const noexcept {
        return size() <= StaticVectorSize;
    }


    template<typename OtherIterator>
    constexpr iterator insert(iterator at, OtherIterator begin, OtherIterator end) {
        AUI_ASSERT_MY_ITERATOR(at);
        auto distance = std::distance(begin, end);

        if (distance + size() <= StaticVectorSize) {
            return inplace()->insert(at, begin, end);
        }
        if (!isInplaceAllocated()) {
            return dynamic()->insert(at, begin, end);
        }

        // switch from static to dynamic
        DynamicVector temp;
        temp.reserve(aui::container::vector_impl::ceilPower2(distance + size()));

        aui::container::vector_impl::insert_no_growth(temp.mEnd, temp.mEnd,
                                                      std::make_move_iterator(this->begin()), std::make_move_iterator(at));

        auto result = aui::container::vector_impl::insert_no_growth(temp.mEnd, temp.mEnd,
                                                                    begin, end);

        aui::container::vector_impl::insert_no_growth(temp.mEnd, temp.mEnd,
                                                      std::make_move_iterator(at), std::make_move_iterator(this->end()));

        inplace()->~StaticVector();
        new (&mBase.dynamic) DynamicVector(std::move(temp));

        return result;
    }

    constexpr iterator erase(iterator begin, iterator end) noexcept {
        AUI_ASSERT_MY_ITERATOR(begin);
        AUI_ASSERT_MY_ITERATOR(end);

        if (isInplaceAllocated()) {
            return inplace()->erase(begin, end);
        }

        if (size() - std::distance(begin, end) > StaticVectorSize) {
            return dynamic()->erase(begin, end);
        }

        auto index = std::distance(this->begin(), begin);

        // switch from dynamic to static
        auto temp = std::move(*dynamic());
        temp.erase(begin, end);
        new (inplace()) StaticVector();
        inplace()->insert(inplace()->end(), std::make_move_iterator(temp.begin()), std::make_move_iterator(temp.end()));
        return this->begin() + index;
    }

    constexpr iterator insert(iterator at, StoredType value) {
        AUI_ASSERT_MY_ITERATOR(at);
        return insert(at, std::make_move_iterator(&value), std::make_move_iterator(&value + 1));
    }

    constexpr iterator erase(iterator at) {
        return erase(at, std::next(at));
    }


    // AUI extensions - see AVector for reference


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


    void sort() noexcept {
        std::sort(super::begin(), super::end());
    }

    template<typename Comparator>
    void sort(Comparator&& comparator) noexcept {
        std::sort(super::begin(), super::end(), std::forward<Comparator>(comparator));
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
        super::erase(std::remove_if(super::begin(), super::end(), std::forward<Predicate>(predicate)), super::end());
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


private:
    union {
        struct {
            StoredType* begin;
            StoredType* end;
        } common;

        std::aligned_storage_t<sizeof(StaticVector), alignof(StaticVector)> inplace;
        std::aligned_storage_t<sizeof(DynamicVector), alignof(DynamicVector)> dynamic;
    } mBase;


    StaticVector* inplace() {
        assert(isInplaceAllocated());
        return reinterpret_cast<StaticVector*>(&mBase.inplace);
    }

    DynamicVector * dynamic() {
        assert(!isInplaceAllocated());
        return reinterpret_cast<DynamicVector*>(&mBase.dynamic);
    }

    void deallocate() {
        if (isInplaceAllocated()) {
            inplace()->~StaticVector();
        } else {
            dynamic()->~DynamicVector();
        }
    }
};
