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

#include <cstddef>
#include <functional>

#include "ADynamicVector.h"
#include "AStaticVector.h"
#include "AUI/Traits/concepts.h"

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
        AUI_ASSERTX(size() > 0, "ASmallVector is empty");
        erase(std::prev(end()));
    }
    constexpr void pop_front() noexcept {
        AUI_ASSERTX(size() > 0, "ASmallVector is empty");
        erase(begin());
    }

    [[nodiscard]]
    constexpr StoredType& operator[](std::size_t index) noexcept {
        AUI_ASSERTX(index < size(), "out of bounds");
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
        new (&mBase.inplace) StaticVector();
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
        temp.reserve(aui::bit_ceil(distance + size()));

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
     * Removes all occurrences of <code>item</code> with specified projection.
     * @param item element to remove.
     * @param projection callable that transforms <code>const StoredType&</code> to <code>const T&</code>. Can be any
     *        operator() cappable object, including lambda and pointer-to-member.
     */
    template<typename T, aui::mapper<const StoredType&, const T&> Projection>
    void removeAll(const T& item, Projection projection) noexcept
    {
        aui::container::remove_all(*this, item, projection);
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
     * @param c container to push
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
     * @param c container to push
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
        AUI_ASSERTX(!super::empty(), "empty container could not have the first element");
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
        AUI_ASSERTX(!super::empty(), "empty container could not have the first element");
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
        AUI_ASSERTX(!super::empty(), "empty container could not have the last element");
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
        AUI_ASSERTX(!super::empty(), "empty container could not have the last element");
        return super::back();
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
        std::sort(super::begin(), super::end());
    }

    template<typename Comparator>
    void sort(Comparator&& comparator) noexcept {
        std::sort(super::begin(), super::end(), std::forward<Comparator>(comparator));
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
     * @brief Finds element by value
     * @param value value
     * @param projection callable that transforms <code>const StoredType&</code> to <code>const T&</code>. Can be any
     *        operator() cappable object, including lambda and pointer-to-member.
     * @return Pointer to the value on which the predicate returned true, nullptr otherwise
     */
    template<typename T, aui::mapper<const StoredType&, const T&> Projection>
    StoredType* findIf(const T& value, Projection&& projection) noexcept
    {
        if (auto i = std::find_if(super::begin(),
                                  super::end(),
                                  [&](const StoredType& s) { return value == std::invoke(projection, s); }
                                  ); i != super::end()) {
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
     * Removes element if `predicate(container[i]) == true`.
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
        AUI_ASSERT(isInplaceAllocated());
        return reinterpret_cast<StaticVector*>(&mBase.inplace);
    }

    DynamicVector * dynamic() {
        AUI_ASSERT(!isInplaceAllocated());
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
