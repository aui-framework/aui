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

//NOLINTBEGIN(modernize-*,cppcoreguidelines-macro-usage,bugprone-macro-parentheses)

/**
 * @brief Forbids object from being optimized out by compiler.
 * @param object Target object. Can be pointer-to-function, pointer-to-member, any variable.
 * @ingroup useful_macros
 * @details
 * Performs compiler hacks in order to prevent the compiler/linker from optimizing out the specified object. It's useful
 * for debugging purposes, when the function is expected to exist to be called.
 *
 * The macro should be called in any function/method which is not being optimized out (i.e. in destructor).
 *
 * Basic usage:
 * ```cpp
 * struct SomeClass {
 * public:
 *   ~SomeClass() {
 *       AUI_NO_OPTIMIZE_OUT(SomeClass::debuggerMethod)
 *   }
 *
 *   int debuggerMethod() {
 *     ...
 *   }
 * }
 * ```
 */
#define AUI_NO_OPTIMIZE_OUT(object) { auto unused = &object ; }

/**
 * @brief Marks the variable as being used.
 * @ingroup useful_macros
 * @details
 * Silences the unused variable compiler warning.
 *
 * Can be used to force `[=]` lambda to capture a variable.
 */
#define AUI_MARK_AS_USED(variable) { (void)variable; }


/**
 * @brief Explicitly denotes a [reactive](react.md) expression.
 * @ingroup useful_macros
 * @ingroup react
 * @details
 * AUI_REACT is a core component of AUI Framework's [reactive](react.md) reactive programming model. It's used to
 * create [reactive](react.md) expressions that automatically update UI elements when their dependent values
 * change.
 *
 * The expression is a C++ expression that depends on AProperty values:
 *
 * ```cpp
 * AUI_REACT(expression)
 * ```
 *
 * # Basic example
 *
 * This creates a label that automatically updates when property `mCounter` changes:
 *
 * <!-- aui:snippet examples/7guis/counter/src/main.cpp counter -->
 *
 * # Formatted label example
 *
 * <!-- aui:snippet examples/7guis/timer/src/main.cpp example -->
 *
 * # Implementation details
 *
 * When used in declarative UI building, `AUI_REACT` creates an instance of `APropertyPrecomputed<T>` behind the scenes,
 * which:
 *
 * 1. Evaluates the expression initially.
 * 2. Sets up observers for all dependent properties.
 * 3. Re-evaluates when dependencies change.
 *
 * The macros itself consists of a lambda syntax with forced `[=]` capture and explicit `decltype(auto)` return type.
 */
#define AUI_REACT(expression) [=]() -> decltype(auto) { return (expression); }

//NOLINTEND(modernize-*,cppcoreguidelines-macro-usage,bugprone-macro-parentheses)
