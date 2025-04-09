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

#include <AUI/Common/AVector.h>
#include <utility>


/**
 * @brief Stack-only vector implementation.
 * @tparam T
 * @tparam COUNT
 */
template<typename T, std::size_t MAX_SIZE>
class [[deprecated("deprecated in favour of AStaticVector")]] AFlatVector {
public:

    ~AFlatVector() {
        destructAll();
    }

    std::size_t size() const noexcept {
        return mSize;
    }

    T& operator[](std::size_t index) noexcept {
        AUI_ASSERTX(index < size(), "index out of bounds");
        return data()[index];
    }

    const T& operator[](std::size_t index) const noexcept {
        AUI_ASSERTX(index < size(), "index out of bounds");
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
        AUI_ASSERTX(mSize < MAX_SIZE, "size exceeded");
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