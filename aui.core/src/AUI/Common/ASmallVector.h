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

#include "AStaticVector.h"
#include "ADynamicVector.h"

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

    ASmallVector() noexcept {
        new (&mBase.inplace) StaticVector();
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
