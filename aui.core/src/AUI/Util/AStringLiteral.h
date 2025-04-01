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


/**
 * @brief Compile-time string literal.
 * @ingroup core
 */
template <char... chars>
struct AStringLiteral
{
    operator const char *() const noexcept
    {
        static const char result[] = { chars..., 0 };
        return result;
    }
};


template<char... chars1, char... chars2>
constexpr AStringLiteral<chars1..., chars2...> operator+(AStringLiteral<chars1...>, AStringLiteral<chars2...>) {
    return {};
}
template <typename T, T... chars>
constexpr AStringLiteral<chars...> operator""_asl() { return { }; }