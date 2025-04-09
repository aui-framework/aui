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


#include <cstdint>
#include <cstddef>
#include <cassert>
#include <AUI/Common/AVector.h>
#include <AUI/Common/AStaticVector.h>

template<typename Vector, typename T>
concept AAnyVector = requires(Vector& vector)
{
    { vector.data() } -> aui::convertible_to<const T*>;
    { vector.size() } -> aui::convertible_to<std::size_t>;
};

/**
 * @deprecated Use std::span instead.
 */
template<typename T>
class AArrayView {
public:
    AArrayView(const T* data, std::size_t count) noexcept : mData(data), mCount(count) {}

    AArrayView(const AAnyVector<T> auto& vector) noexcept: mData(vector.data()), mCount(vector.size()) {

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
        AUI_ASSERTX(index < mCount, "out of bounds");
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

template<typename Vector>
AArrayView(const Vector& vector) noexcept -> AArrayView<std::decay_t<decltype(*vector.data())>>;
