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

#include "AVector.h"
#include "AException.h"
#include <utility>
#include <span>


#define AUI_ASSERT_MY_ITERATOR(it) AUI_ASSERTX((this->begin() <= it && it <= this->end()), "foreign iterator")

/**
 * @brief Vector-like container up to maxSize elements inplace.
 * @tparam T stored type
 * @tparam maxSize maximum vector size
 * @ingroup core
 *
 * @details
 * Vector-like container optimized for the case when it contains up to maxSize in place, avoiding dynamic
 * allocation. AStaticVector could not contain more than maxSize elements.
 */
template<typename StoredType, std::size_t MaxSize>
class AStaticVector {
public:
    using self = AStaticVector;
    using super = AStaticVector;
    using iterator = StoredType*;
    using const_iterator = const StoredType*;
    using reference = StoredType&;
    using const_reference = const StoredType&;
    using value = StoredType;

    constexpr AStaticVector() noexcept: mBegin(reinterpret_cast<StoredType*>(&mStorage)), mEnd(mBegin) {}
    constexpr AStaticVector(const AStaticVector& rhs): AStaticVector() {
        insert(mBegin, rhs.begin(), rhs.end());
    }
    constexpr AStaticVector(AStaticVector&& rhs) noexcept: AStaticVector() {
        insert(mBegin, std::make_move_iterator(rhs.begin()), std::make_move_iterator(rhs.end()));
        rhs.clear();
    }
    constexpr AStaticVector(std::initializer_list<StoredType> rhs) noexcept: AStaticVector() {
        insert(mBegin, std::make_move_iterator(rhs.begin()), std::make_move_iterator(rhs.end()));
    }
    template<typename Iterator>
    constexpr AStaticVector(Iterator begin, Iterator end) noexcept: AStaticVector() {
        for (auto it = begin; it != end; ++it) { // range-v3 fix: basic range traversal instead of insert
            push_back(*it);
        }
    }
    constexpr ~AStaticVector() {
        for (auto& v : *this) {
            v.~StoredType();
        }
    }

    [[nodiscard]]
    static constexpr size_t capacity() noexcept {
        return MaxSize;
    }

    AStaticVector& operator=(const AStaticVector& rhs) {
        if (this == &rhs) {
            return *this;
        }
        clear();
        insert(mBegin, rhs.begin(), rhs.end());
        return *this;
    }

    AStaticVector& operator=(AStaticVector&& rhs) noexcept {
        if (this == &rhs) {
            return *this;
        }
        clear();
        insert(mBegin, std::make_move_iterator(rhs.begin()), std::make_move_iterator(rhs.end()));
        rhs.clear();
        return *this;
    }

    [[nodiscard]]
    constexpr bool full() const noexcept {
        return size() >= MaxSize;
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

    template<typename... Args>
    constexpr void emplace_back(Args&&... args) noexcept {
        AUI_ASSERTX(!full(), "insufficient size in AStaticVector");
        new (mEnd++) StoredType(std::forward<Args>(args)...);
    }


    constexpr void push_back(StoredType value) noexcept {
        AUI_ASSERTX(!full(), "insufficient size in AStaticVector");
        new (mEnd++) StoredType(std::move(value));
    }

    constexpr void push_front(StoredType value) noexcept {
        AUI_ASSERTX(!full(), "insufficient size in AStaticVector");
        insert(begin(), std::move(value));
    }

    constexpr void pop_back() noexcept {
        AUI_ASSERTX(size() > 0, "AStaticVector is empty");
        erase(std::prev(end()));
    }
    constexpr void pop_front() noexcept {
        AUI_ASSERTX(size() > 0, "AStaticVector is empty");
        erase(begin());
    }

    [[nodiscard]]
    constexpr StoredType& operator[](std::size_t index) noexcept {
        AUI_ASSERTX(index < size(), "out of bounds");
        return *(data() + index);
    }

    [[nodiscard]]
    constexpr StoredType& operator[](std::size_t index) const noexcept {
        return const_cast<AStaticVector*>(this)->operator[](index);
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept {
        return begin() == end();
    }

    constexpr void clear() noexcept {
        for (auto& v : *this) {
            v.~StoredType();
        }
        mEnd = mBegin;
    }

    [[nodiscard]]
    constexpr std::size_t size() const noexcept {
        return mEnd - mBegin;
    }

    template<typename OtherIterator>
    constexpr iterator insert(iterator at, OtherIterator begin, OtherIterator end) {
        AUI_ASSERT_MY_ITERATOR(at);
        auto distance = std::distance(begin, end);
        AUI_ASSERTX(size() + distance <= MaxSize, "out of bounds");

        return aui::container::vector_impl::insert_no_growth(mEnd, at, begin, end);
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

    void reserve(std::size_t) {
        // does nothing - just for std::vector compatibility
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
        result.reserve(super::size());
        for (const auto& element : *this) {
            if (predicate(element)) {
                result.push_back(element);
            }
        }
        return result;
    }

    [[nodiscard]]
    operator std::span<StoredType>() {
        return std::span(data(), size());
    }

    [[nodiscard]]
    operator std::span<const StoredType>() const {
        return std::span(data(), size());
    }

private:
    iterator mBegin;
    iterator mEnd;
    std::aligned_storage_t<sizeof(StoredType) * MaxSize, alignof(StoredType)> mStorage;
};
