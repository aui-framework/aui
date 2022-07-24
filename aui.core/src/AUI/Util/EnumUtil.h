/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#pragma once

#include <AUI/Util/ABitField.h>


/**
 * @brief Provides bitwise operators for enum classes.
 * @ingroup useful_macros
 * @details
 * Unlike regular enums, enum classes are not supporting bitwise operators. This macro provides them using ABitField
 * helper class.
 * @code{cpp}
 * AUI_ENUM_FLAG(Flags) {
 *     FLAG1 = 1 << 0,
 *     FLAG2 = 1 << 1,
 *     FLAG3 = 1 << 2,
 *     FLAG23 = FLAG2 | FLAG3, // even here flag combination is possible
 * };
 * ...
 * Flags f = Flags::FLAG1 | Flags::FLAG2;
 * if (f & Flags::FLAG1) ...  // true
 * if (f & Flags::FLAG2) ...  // true
 * if (f & Flags::FLAG3) ...  // false
 * if (f & Flags::FLAG23) ... // true
 *
 * @endcode
 */
#define AUI_ENUM_FLAG(name) enum class name: int; \
                            constexpr inline ABitField<name> operator|(name a, name b) {return static_cast<name>(static_cast<int>(a) | static_cast<int>(b));} \
                            constexpr inline ABitField<name> operator&(name a, name b) {return static_cast<name>(static_cast<int>(a) & static_cast<int>(b));} \
                            constexpr inline ABitField<name> operator^(name a, name b) {return static_cast<name>(static_cast<int>(a) ^ static_cast<int>(b));} \
                            constexpr inline ABitField<name> operator|=(name& a, name b) {return a = a | b;} \
                            constexpr inline ABitField<name> operator&=(name& a, name b) {return a = a & b;} \
                            constexpr inline ABitField<name> operator^=(name& a, name b) {return a = a ^ b;} \
                            constexpr inline ABitField<name> operator~(name a) {return static_cast<name>(~static_cast<int>(a));} \
                            enum class name: int

#define AUI_ENUM_INT(name) enum class name: int; \
                           constexpr inline bool operator<(name a, name b) {return static_cast<int>(a) < static_cast<int>(b);} \
                           constexpr inline bool operator>(name a, name b) {return static_cast<int>(a) > static_cast<int>(b);} \
                           enum class name: int
