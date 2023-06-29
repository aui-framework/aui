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