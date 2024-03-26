// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include <cassert>
#include "AUI/api.h"
#include "AUI/Util/APreprocessor.h"

namespace aui::assertion::detail {
    inline void checkArgs(bool cond, const char* what = nullptr) {}

    API_AUI_CORE void triggerAssertion(const char* message);
}



#if AUI_DEBUG
#define AUI_IMPL_ASSERT(cond) assert(cond); ::aui::assertion::detail::checkArgs(cond)
#define AUI_IMPL_ASSERTX(cond, what) assert((cond) && what); ::aui::assertion::detail::checkArgs(cond, what)
#else
#define AUI_IMPL_ASSERT(cond) if (!(cond)) ::aui::assertion::detail::triggerAssertion("assertion failed: " AUI_PP_STRINGIZE(cond))
#define AUI_IMPL_ASSERTX(cond, what) if (!(cond)) ::aui::assertion::detail::triggerAssertion("assertion failed: " AUI_PP_STRINGIZE(cond) ": " what); ::aui::assertion::detail::checkArgs(cond, what)
#endif

/**
 * @brief Asserts that the passed condition evaluates to true.
 * @param condition the expression
 * @ingroup useful_macros
 * @details
 * If the condition evaluates to false, triggers default C++ assert behavior (that is, program termination) on debug
 * build or throws AAssertionFailedException on release builds, so it can be handled and reported properly in production
 * applications.
 * @sa AUI_ASSERTX
 *
 * @code{cpp}
 * int v = 2 + 2;
 * AUI_ASSERT(v >= 0); // does not trigger
 * AUI_ASSERT(v != 4); // triggers
 * @endcode
 */
#define AUI_ASSERT(condition) AUI_IMPL_ASSERT(condition)

/**
 * @brief Asserts that the passed condition evaluates to true. Adds extra message string.
 * @param condition the expression
 * @param what string literal which will be appended to the error message
 * @ingroup useful_macros
 * @details
 * If the condition evaluates to false, triggers default C++ assert behavior (that is, program termination) on debug
 * build or throws AAssertionFailedException on release builds, so it can be handled and reported properly in production
 * applications.
 * @sa AUI_ASSERT
 *
 * @code{cpp}
 * int v = 2 + 2;
 * AUI_ASSERTX(v >= 0, "positive value expected"); // does not trigger
 * AUI_ASSERTX(v != 4, "4 is unacceptable value"); // triggers
 * @endcode
 */
#define AUI_ASSERTX(condition, what) AUI_IMPL_ASSERTX(condition, what)