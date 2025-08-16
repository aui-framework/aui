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

#include <utility>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/integer.hpp>

#include "AVector.h"
#include "AException.h"

#define AUI_ASSERT_MY_ITERATOR(it) AUI_ASSERTX((this->begin() <= it && it <= this->end()), "foreign iterator")

/**
 * @brief Vector implementation for ASmallVector.
 * @tparam T stored type
 * @details
 * !!! note
 *
 *     This vector implementation is indented to use only by ASmallVector. If it's not your case, consider using AVector
 *     instead.
 */
template<typename StoredType>
class ADynamicVector {
public:
    using self = ADynamicVector;
    using super = ADynamicVector;
    using iterator = StoredType*;
    using const_iterator = const StoredType*;
    using reference = StoredType&;
    using const_reference = const StoredType&;
    using value = StoredType;

    constexpr ADynamicVector() noexcept {}
    constexpr ADynamicVector(const ADynamicVector& rhs): ADynamicVector() {
        insert(mBegin, rhs.begin(), rhs.end());
    }
    constexpr ADynamicVector(ADynamicVector&& rhs) noexcept: ADynamicVector() {
        operator=(std::move(rhs));
    }
    constexpr ~ADynamicVector() {
        deallocate();
    }

    [[nodiscard]]
    constexpr StoredType* data() noexcept {
        return mBegin;
    }
    [[nodiscard]]
    constexpr const StoredType* data() const noexcept {
        return mBegin;
    }

    [[nodiscard]]
    constexpr iterator begin() noexcept {
        return mBegin;
    }
    [[nodiscard]]
    constexpr const_iterator begin() const noexcept {
        return mBegin;
    }

    [[nodiscard]]
    constexpr iterator end() noexcept {
        return mEnd;
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept {
        return mEnd;
    }

    [[nodiscard]]
    constexpr StoredType& front() noexcept {
        return *begin();
    }

    [[nodiscard]]
    constexpr StoredType& back() noexcept {
        return *std::prev(end());
    }

    [[nodiscard]]
    constexpr const StoredType& front() const noexcept {
        return *begin();
    }

    [[nodiscard]]
    constexpr const StoredType& back() const noexcept {
        return *std::prev(end());
    }

    constexpr void push_back(StoredType value) noexcept {
        insert(end(), std::move(value));
    }

    constexpr void push_front(StoredType value) noexcept {
        insert(begin(), std::move(value));
    }

    constexpr void pop_back() noexcept {
        AUI_ASSERTX(size() > 0, "ADynamicVector is empty");
        erase(std::prev(end()));
    }
    constexpr void pop_front() noexcept {
        AUI_ASSERTX(size() > 0, "ADynamicVector is empty");
        erase(begin());
    }

    [[nodiscard]]
    constexpr StoredType& operator[](std::size_t index) noexcept {
        AUI_ASSERTX(index < size(), "out of bounds");
        return *(data() + index);
    }

    [[nodiscard]]
    constexpr StoredType& operator[](std::size_t index) const noexcept {
        return const_cast<ADynamicVector*>(this)->operator[](index);
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept {
        return begin() == end();
    }

    constexpr void clear() noexcept {
        deallocate();
        mBegin = mEnd = mBufferEnd = nullptr;
    }

    void deallocate() {
        for (auto& v : *this) {
            v.~StoredType();
        }
        operator delete[](mBegin);
    }

    [[nodiscard]]
    constexpr std::size_t size() const noexcept {
        return mEnd - mBegin;
    }

    [[nodiscard]]
    constexpr std::size_t reserved() const noexcept {
        return mBufferEnd - mBegin;
    }

    template<typename OtherIterator>
    constexpr iterator insert(iterator at, OtherIterator begin, OtherIterator end) {
        AUI_ASSERT_MY_ITERATOR(at);
        auto distance = std::distance(begin, end);

        if (size() + distance <= reserved()) {
            return aui::container::vector_impl::insert_no_growth(mEnd, at, begin, end);
        }
        ADynamicVector temp;
        temp.reserve(aui::bit_ceil(distance + size()));
        aui::container::vector_impl::insert_no_growth(temp.mEnd, temp.mEnd,
                                                      std::make_move_iterator(mBegin), std::make_move_iterator(at));

        auto result = aui::container::vector_impl::insert_no_growth(temp.mEnd, temp.mEnd,
                                                                    begin, end);

        aui::container::vector_impl::insert_no_growth(temp.mEnd, temp.mEnd,
                                                      std::make_move_iterator(at), std::make_move_iterator(mEnd));
        operator=(std::move(temp));

        return result;

    }

    constexpr iterator insert(iterator at, StoredType value) {
        AUI_ASSERT_MY_ITERATOR(at);
        return insert(at, std::make_move_iterator(&value), std::make_move_iterator(&value + 1));
    }

    constexpr iterator erase(iterator at) {
        return erase(at, std::next(at));
    }

    constexpr iterator erase(iterator begin, iterator end) {
        AUI_ASSERT_MY_ITERATOR(begin);
        AUI_ASSERT_MY_ITERATOR(end);

        return aui::container::vector_impl::erase(mBegin, mEnd, begin, end);
    }

    void reserve(std::size_t newSize) {
        if (reserved() == newSize) {
            return;
        }

        auto newBuffer = static_cast<StoredType*>(operator new[](newSize * sizeof(StoredType)));
        auto newBufferEnd = newBuffer;
        try {
            auto elementsToMove = std::min(newSize, size());
            auto moveFrom = begin();
            for (std::size_t i = 0; i < elementsToMove; ++newBufferEnd, ++moveFrom) {
                new (newBufferEnd) StoredType(std::move(*moveFrom));
            }
            deallocate();
            mBegin = newBuffer;
            mEnd = newBufferEnd;
            mBufferEnd = newBuffer + newSize;
        } catch (...) { // unlikely; but just in case
            delete[] newBuffer;
            throw;
        }
    }

    ADynamicVector& operator=(ADynamicVector&& rhs) noexcept {
        deallocate();
        mBegin     = rhs.mBegin;
        mEnd       = rhs.mEnd;
        mBufferEnd = rhs.mBufferEnd;

        rhs.mBegin     = nullptr;
        rhs.mEnd       = nullptr;
        rhs.mBufferEnd = nullptr;

        return *this;
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
     * Removes element if `predicate(container[i])` == true.
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


    iterator mBegin = nullptr;
    iterator mEnd = nullptr;
    iterator mBufferEnd = nullptr;
};
