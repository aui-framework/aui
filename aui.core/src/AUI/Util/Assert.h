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

#include <cassert>
#include "AUI/api.h"
#include "AUI/Util/APreprocessor.h"

//NOLINTBEGIN(modernize-*,cppcoreguidelines-macro-usage,bugprone-macro-parentheses)

namespace aui::assertion::detail {
    inline void checkArgs(bool cond, const char* what = nullptr) {}

    API_AUI_CORE void triggerAssertion(const char* message);
}



#if AUI_DEBUG
#define AUI_IMPL_ASSERT(cond) assert(cond); ::aui::assertion::detail::checkArgs(cond)
#define AUI_IMPL_ASSERTX(cond, what) assert((cond) && what); ::aui::assertion::detail::checkArgs(cond, what)
#define AUI_IMPL_FAIL(what) assert(false && what); ::aui::assertion::detail::checkArgs(false, what)
#else
#define AUI_IMPL_ASSERT(cond) if (!(cond)) ::aui::assertion::detail::triggerAssertion("assertion failed: " AUI_PP_STRINGIZE(cond))
#define AUI_IMPL_ASSERTX(cond, what) if (!(cond)) ::aui::assertion::detail::triggerAssertion("assertion failed: " AUI_PP_STRINGIZE(cond) ": " what); ::aui::assertion::detail::checkArgs(cond, what)
#define AUI_IMPL_FAIL(what) ::aui::assertion::detail::triggerAssertion(what); ::aui::assertion::detail::checkArgs(false, what)
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
 * @sa AUI_ASSERT_NO_CONDITION
 *
 * ```cpp
 * int v = 2 + 2;
 * AUI_ASSERT(v >= 0); // does not trigger
 * AUI_ASSERT(v != 4); // triggers
 * ```
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
 * ```cpp
 * int v = 2 + 2;
 * AUI_ASSERTX(v >= 0, "positive value expected"); // does not trigger
 * AUI_ASSERTX(v != 4, "4 is unacceptable value"); // triggers
 * ```
 */
#define AUI_ASSERTX(condition, what) AUI_IMPL_ASSERTX(condition, what)

/**
 * @brief Always triggers assertion fail.
 * @param what string literal which will be appended to the error message
 * @ingroup useful_macros
 * @details
 * Triggers default C++ assert behavior (that is, program termination) on debug build or throws
 * AAssertionFailedException on release builds, so it can be handled and reported properly in production
 * applications.
 * @sa AUI_ASSERT
 *
 * ```cpp
 * switch (i) {
 *   case 0: // ...
 *   default:
 *     AUI_ASSERT_NO_CONDITION("invalid i");
 * }
 * ```
 */
#define AUI_ASSERT_NO_CONDITION(what) AUI_IMPL_FAIL(what)

//NOLINTEND(modernize-*,cppcoreguidelines-macro-usage,bugprone-macro-parentheses)
