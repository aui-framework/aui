// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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


#define AUI_ENUM_FLAG(name) enum class name: int; \
                            constexpr inline name operator|(name a, name b) {return static_cast<name>(static_cast<int>(a) | static_cast<int>(b));} \
                            constexpr inline name operator&(name a, name b) {return static_cast<name>(static_cast<int>(a) & static_cast<int>(b));} \
                            constexpr inline name operator^(name a, name b) {return static_cast<name>(static_cast<int>(a) ^ static_cast<int>(b));} \
                            constexpr inline name operator|=(name& a, name b) {return a = static_cast<name>(static_cast<int>(a) | static_cast<int>(b));} \
                            constexpr inline name operator&=(name& a, name b) {return a = static_cast<name>(static_cast<int>(a) & static_cast<int>(b));} \
                            constexpr inline name operator^=(name& a, name b) {return a = static_cast<name>(static_cast<int>(a) ^ static_cast<int>(b));} \
                            constexpr inline name operator~(const name& a) {return static_cast<name>(~static_cast<int>(a));} \
                            constexpr inline bool operator!(const name& a) {return a == static_cast<name>(0);}                                     \
                                                                   \
                            constexpr inline bool operator&&(const name& a, bool v) {return static_cast<int>(a) && v;}                                     \
                            constexpr inline bool operator||(const name& a, bool v) {return static_cast<int>(a) || v;}                                     \
\
                            enum class name: int

#define AUI_ENUM_INT(name) enum class name: int; \
                           constexpr inline bool operator<(name a, name b) {return static_cast<int>(a) < static_cast<int>(b);} \
                           constexpr inline bool operator>(name a, name b) {return static_cast<int>(a) > static_cast<int>(b);} \
                           enum class name: int
