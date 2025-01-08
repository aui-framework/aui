/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

// android again lacks C++20 features.
// this header implements #include <bit> (notably, std::bit_ceil)

#include <concepts>
#include <bit>
#include "concepts.h"

namespace aui {

template <aui::unsigned_integral T>
    requires(!aui::same_as<T, bool>) && (!aui::same_as<T, char>) && (!aui::same_as<T, char8_t>) &&
            (!aui::same_as<T, char16_t>) && (!aui::same_as<T, char32_t>) && (!aui::same_as<T, wchar_t>)
constexpr T bit_width(T x) noexcept {
    return std::numeric_limits<T>::digits - std::countl_zero(x);
}

template <aui::unsigned_integral T>
    requires(!aui::same_as<T, bool>) && (!aui::same_as<T, char>) && (!aui::same_as<T, char8_t>) &&
            (!aui::same_as<T, char16_t>) && (!aui::same_as<T, char32_t>) && (!aui::same_as<T, wchar_t>)
constexpr T bit_ceil(T x) noexcept {
    if (x <= 1u)
        return T(1);
    if constexpr (aui::same_as<T, decltype(+x)>)
        return T(1) << aui::bit_width(T(x - 1));
    else {   // for types subject to integral promotion
        constexpr int offset_for_ub = std::numeric_limits<unsigned>::digits - std::numeric_limits<T>::digits;
        return T(1u << (aui::bit_width(T(x - 1)) + offset_for_ub) >> offset_for_ub);
    }
}

template <aui::unsigned_integral T>
    requires(!std::same_as<T, bool>) && (!std::same_as<T, char>) && (!std::same_as<T, char8_t>) &&
            (!std::same_as<T, char16_t>) && (!std::same_as<T, char32_t>) && (!std::same_as<T, wchar_t>)
constexpr T bit_floor(T x) noexcept {
    if (x != 0)
        return T { 1 } << (aui::bit_width(x) - 1);
    return 0;
}

}   // namespace aui
