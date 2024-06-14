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
