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

#include <AUI/Common/AVector.h>
#include <utility>


/**
 * @brief Stack-only vector implementation.
 * @tparam T
 * @tparam COUNT
 */
template<typename T, std::size_t MAX_SIZE>
class AFlatVector {
public:

    ~AFlatVector() {
        destructAll();
    }

    std::size_t size() const noexcept {
        return mSize;
    }

    T& operator[](std::size_t index) noexcept {
        assert(("index out of bounds", index < size()));
        return data()[index];
    }

    const T& operator[](std::size_t index) const noexcept {
        assert(("index out of bounds", index < size()));
        return data()[index];
    }

    [[nodiscard]]
    T& at(std::size_t index) {
        if (index >= size()) {
            aui::impl::outOfBoundsException();
        }
        return operator[](index);
    }

    [[nodiscard]]
    const T& at(std::size_t index) const {
        if (index >= size()) {
            aui::impl::outOfBoundsException();
        }
        return operator[](index);
    }

    T* data() noexcept {
        return reinterpret_cast<T*>(&mStorage);
    }

    const T* data() const noexcept {
        return reinterpret_cast<const T*>(&mStorage);
    }

    void clear() {
        destructAll();
        mSize = 0;
    }

    void push_back(T value) {
        assert(("size exceeded", mSize < MAX_SIZE));
        new (data() + mSize) T(std::move(value));
    }

    AFlatVector& operator<<(T value) {
        push_back(std::move(value));
        return *this;
    }



private:
    std::size_t mSize = 0;
    std::aligned_storage_t<sizeof(T) * MAX_SIZE, alignof(T)> mStorage;

    void destructAll() {
        for (size_t i = 0; i < size(); ++i) {
            data()[i].~T();
        }
    }
};