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

public:
    /*
    ASmallVector() noexcept {
        new (inplace()) StaticVector();
    }

    ~ASmallVector() noexcept {
        if (isInplaceAllocated()) {
            inplace()->~StaticVector();
        }
    }


    [[nodiscard]]
    constexpr StoredType* data() noexcept {
        return reinterpret_cast<StoredType*>(&mStorage);
    }
    [[nodiscard]]
    constexpr const StoredType* data() const noexcept {
        return reinterpret_cast<const StoredType*>(&mStorage);
    }

    [[nodiscard]]
    constexpr iterator begin() noexcept {
        return data();
    }
    [[nodiscard]]
    constexpr const_iterator begin() const noexcept {
        return data();
    }

    [[nodiscard]]
    constexpr iterator end() noexcept {
        return data() + mSize;
    }

    [[nodiscard]]
    constexpr const_iterator end() const noexcept {
        return data() + mSize;
    }

    [[nodiscard]]
    constexpr StoredType& front() noexcept {
        return *begin();
    }

    [[nodiscard]]
    constexpr StoredType& back() noexcept {
        return *(begin() + (mSize - 1));
    }

    [[nodiscard]]
    constexpr const StoredType& front() const noexcept {
        return *begin();
    }

    [[nodiscard]]
    constexpr const StoredType& back() const noexcept {
        return *(begin() + (mSize - 1));
    }

    constexpr void push_back(StoredType value) noexcept {
        assert(("insufficient size in AStaticVector", mSize + 1 <= MaxSize));
        new (data() + mSize++) StoredType(std::move(value));
    }

    constexpr void push_front(StoredType value) noexcept {
        assert(("insufficient size in AStaticVector", mSize + 1 <= MaxSize));
        insert(begin(), std::move(value));
    }

    constexpr void pop_back() noexcept {
        assert(("AStaticVector is empty", mSize > 0));
        erase(std::prev(end()));
    }
    constexpr void pop_front() noexcept {
        assert(("AStaticVector is empty", mSize > 0));
        erase(begin());
    }

    [[nodiscard]]
    constexpr StoredType& operator[](std::size_t index) noexcept {
        assert(("out of bounds", index < mSize));
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
        mSize = 0;
    }
*/
    /**
     * @return Size of the container.
     */
    [[nodiscard]]
    std::size_t size() const noexcept {
        return mBase.safe.validElementCount;
    }


    [[nodiscard]]
    bool isInplaceAllocated() const noexcept {
        return size() <= StaticVectorSize;
    }

private:

    StaticVector* inplace() {
        return reinterpret_cast<StaticVector*>(&mBase.inplace);
    }

    union {
        struct {
            std::size_t validElementCount;
        } safe;

        struct {
            std::size_t validElementCount;
            std::size_t reservedElementCount;
            StoredType* buffer;
        } dynamic;

        std::aligned_storage_t<sizeof(StaticVector), alignof(StaticVector)> inplace;
    } mBase;
};
