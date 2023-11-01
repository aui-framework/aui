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


#include <cstdint>
#include <cstddef>
#include <cassert>
#include <AUI/Common/AVector.h>

/**
 * @deprecated Use std::span instead.
 */
template<typename T>
class AArrayView {
public:
    AArrayView(const T* data, std::size_t count) noexcept : mData(data), mCount(count) {}
    AArrayView(const AVector<T>& vector) noexcept: mData(vector.data()), mCount(vector.size()) {

    }

    template<std::size_t N>
    AArrayView(const T (&rawArray)[N]) noexcept: mData(rawArray), mCount(N) {}

    template<std::size_t N>
    AArrayView(const std::array<T, N>& array) noexcept: mData(array.data()), mCount(N) {}

    [[nodiscard]]
    const T* data() const noexcept {
        return mData;
    }

    [[nodiscard]]
    size_t size() const noexcept {
        return mCount;
    }
    [[nodiscard]]
    size_t sizeInBytes() const noexcept {
        return mCount * sizeof(T);
    }

    const T& operator[](std::size_t index) const noexcept {
        assert(("out of bounds", index < mCount));
        return mData[index];
    }

    const T* begin() const noexcept {
        return mData;
    }

    const T* end() const noexcept {
        return mData + mCount;
    }

private:
    const T* mData;
    std::size_t mCount;
};