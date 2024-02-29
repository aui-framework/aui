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
 * @code{cpp}
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
 * @endcode
 */
#define AUI_NO_OPTIMIZE_OUT(object) { auto unused = &object ; }

/**
 * @brief Marks the variable as being used.
 * @ingroup useful_macros
 * @details
 * Silences the unused variable compiler warning.
 *
 * Can be used to force [=] lambda to capture a variable.
 */
#define AUI_MARK_AS_USED(variable) { (void)variable; }