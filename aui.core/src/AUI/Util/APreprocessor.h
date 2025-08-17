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

// Code from several projects used here, such as boost and gtest.

//NOLINTBEGIN(modernize-*,cppcoreguidelines-macro-*)

/**
 * @brief Expands and concatenates the arguments. Constructed macros reevaluate.
 * @ingroup useful_macros
 */
#define AUI_PP_CAT(_1, _2) AUI_PP_INTERNAL_CAT(_1, _2)

/**
 * @brief Expands and stringifies the only argument.
 * @ingroup useful_macros
 */
#define AUI_PP_STRINGIZE(...) AUI_PP_INTERNAL_STRINGIZE(__VA_ARGS__)

/**
 * @brief Returns empty. Given a variadic number of arguments.
 * @ingroup useful_macros
 */
#define AUI_PP_EMPTY(...)

/**
 * @brief Returns a comma. Given a variadic number of arguments.
 * @ingroup useful_macros
 */
#define AUI_PP_COMMA(...) ,

/**
 * @brief Returns the only argument.
 * @ingroup useful_macros
 */
#define AUI_PP_IDENTITY(_1) _1

/**
 * @brief Evaluates to the number of arguments after expansion.
 * @details
 * ```cpp
 * #define PAIR x, y
 *
 * AUI_PP_NARG() => 1
 * AUI_PP_NARG(x) => 1
 * AUI_PP_NARG(x, y) => 2
 * AUI_PP_NARG(PAIR) => 2
 * ```
 *
 * Requires: the number of arguments after expansion is at most 15.
 * @ingroup useful_macros
 */
#define AUI_PP_NARG(...) \
  AUI_PP_INTERNAL_16TH(  \
      (__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

/**
 * @brief Returns 1 if the expansion of arguments has an unprotected comma.Otherwise returns 0.
 * @details Requires no more than 15 unprotected commas.
 * @ingroup useful_macros
 */
#define AUI_PP_HAS_COMMA(...) \
  AUI_PP_INTERNAL_16TH(       \
      (__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0))

/**
 * @brief Returns the first argument.
 * @ingroup useful_macros
 */
#define AUI_PP_HEAD(...) AUI_PP_INTERNAL_HEAD((__VA_ARGS__, unusedArg))

/**
 * @brief Returns the tail. A variadic list of all arguments minus the first
 * @details
 * Requires at least one argument.
 * @ingroup useful_macros
 */
#define AUI_PP_TAIL(...) AUI_PP_INTERNAL_TAIL((__VA_ARGS__))

/**
 * @brief Calls CAT(_Macro, NARG(__VA_ARGS__))(__VA_ARGS__)
 * @ingroup useful_macros
 */
#define AUI_PP_VARIADIC_CALL(_Macro, ...) \
  AUI_PP_IDENTITY(                        \
      AUI_PP_CAT(_Macro, AUI_PP_NARG(__VA_ARGS__))(__VA_ARGS__))

/**
 * @brief If the arguments after expansion have no tokens, evaluates to `1`. Otherwise evaluates to `0`.
 * @details
 * Requires:
 * - the number of arguments after expansion is at most 15.
 * - If the argument is a macro, it must be able to be called with one argument.
 *
 * ## Implementation details
 *
 * There is one case when it generates a compile error: if the argument is macro
 * that cannot be called with one argument.
 *
 * ```cpp
 * #define M(a, b)  // it doesn't matter what it expands to
 *
 * // Expected: expands to `0`.
 * // Actual: compile error.
 * AUI_PP_IS_EMPTY(M)
 * ```
 *
 * There are 4 cases tested:
 *
 * - `__VA_ARGS__` possible expansion has no unparen'd commas. Expected 0.
 * - `__VA_ARGS__` possible expansion is not enclosed in parenthesis. Expected 0.
 * - `__VA_ARGS__` possible expansion is not a macro that ()-evaluates to a comma. Expected 0
 * - `__VA_ARGS__` is empty, or has unparen'd commas, or is enclosed in parenthesis, or is a macro that ()-evaluates to comma. Expected 1.
 *
 * We trigger detection on '0001', i.e. on empty.
 * @ingroup useful_macros
 */
#define AUI_PP_IS_EMPTY(...)                                               \
  AUI_PP_INTERNAL_IS_EMPTY(AUI_PP_HAS_COMMA(__VA_ARGS__),                \
                             AUI_PP_HAS_COMMA(AUI_PP_COMMA __VA_ARGS__), \
                             AUI_PP_HAS_COMMA(__VA_ARGS__()),              \
                             AUI_PP_HAS_COMMA(AUI_PP_COMMA __VA_ARGS__()))

/**
 * @brief Evaluates to _Then if _Cond is 1 and _Else if _Cond is 0.
 * @ingroup useful_macros
 */
#define AUI_PP_IF(_Cond, _Then, _Else) \
  AUI_PP_CAT(AUI_PP_INTERNAL_IF_, _Cond)(_Then, _Else)

/**
 * @brief Similar to AUI_PP_IF but takes _Then and _Else in parentheses.
 * @details
 * ```cpp
 * AUI_PP_GENERIC_IF(1, (a, b, c), (d, e, f)) => a, b, c
 * AUI_PP_GENERIC_IF(0, (a, b, c), (d, e, f)) => d, e, f
 * ```
 * @ingroup useful_macros
 */
#define AUI_PP_GENERIC_IF(_Cond, _Then, _Else) \
  AUI_PP_REMOVE_PARENS(AUI_PP_IF(_Cond, _Then, _Else))

/**
 * @brief Evaluates to the number of arguments after expansion. Identifies 'empty' as 0.
 * @details
 * ```cpp
 * #define PAIR x, y
 *
 * AUI_PP_NARG0() => 0
 * AUI_PP_NARG0(x) => 1
 * AUI_PP_NARG0(x, y) => 2
 * AUI_PP_NARG0(PAIR) => 2
 * ```
 *
 * Requires:
 * - the number of arguments after expansion is at most 15.
 * - If the argument is a macro, it must be able to be called with one argument.
 * @ingroup useful_macros
 */
#define AUI_PP_NARG0(...) \
  AUI_PP_IF(AUI_PP_IS_EMPTY(__VA_ARGS__), 0, AUI_PP_NARG(__VA_ARGS__))

/**
 * @brief Expands to 1 if the first argument starts with something in parentheses, otherwise to 0.
 * @ingroup useful_macros
 */
#define AUI_PP_IS_BEGIN_PARENS(...)                              \
  AUI_PP_HEAD(AUI_PP_CAT(AUI_PP_INTERNAL_IBP_IS_VARIADIC_R_, \
                             AUI_PP_INTERNAL_IBP_IS_VARIADIC_C __VA_ARGS__))

/**
 * @brief Expands to 1 is there is only one argument and it is enclosed in parentheses.
 * @ingroup useful_macros
 */
#define AUI_PP_IS_ENCLOSED_PARENS(...)             \
  AUI_PP_IF(AUI_PP_IS_BEGIN_PARENS(__VA_ARGS__), \
              AUI_PP_IS_EMPTY(AUI_PP_EMPTY __VA_ARGS__), 0)

/**
 * @brief Remove the parens, requires AUI_PP_IS_ENCLOSED_PARENS(args) => 1.
 * @ingroup useful_macros
 */
#define AUI_PP_REMOVE_PARENS(...) AUI_PP_INTERNAL_REMOVE_PARENS __VA_ARGS__

/**
 * @brief Expands to _Macro(0, _Data, e1) _Macro(1, _Data, e2) ... _Macro(K -1, _Data, eK) as many of AUI_INTERNAL_NARG0 _Tuple.
 * @details
 * Requires:
 * - `_Macro` can be called with 3 arguments.
 * - `_Tuple` expansion has no more than 15 elements.
 * @ingroup useful_macros
 */
#define AUI_PP_FOR_EACH(_Macro, _Data, _Tuple)                        \
  AUI_PP_CAT(AUI_PP_INTERNAL_FOR_EACH_IMPL_, AUI_PP_NARG0 _Tuple) \
  (0, _Macro, _Data, _Tuple)

/**
 * @brief Expands to _Macro(0, _Data, ) _Macro(1, _Data, ) ... _Macro(K - 1, _Data, )
 * @details
 * Empty if `_K = 0`.
 * Requires:
 * - `_Macro` can be called with 3 arguments.
 * - `_K` literal between 0 and 15.
 * @ingroup useful_macros
 */
#define AUI_PP_REPEAT(_Macro, _Data, _N)           \
  AUI_PP_CAT(AUI_PP_INTERNAL_FOR_EACH_IMPL_, _N) \
  (0, _Macro, _Data, AUI_PP_INTENRAL_EMPTY_TUPLE)

/**
 * @brief Increments the argument, requires the argument to be between 0 and 15.
 * @ingroup useful_macros
 */
#define AUI_PP_INC(_i) AUI_PP_CAT(AUI_PP_INTERNAL_INC_, _i)

/**
 * @brief Returns comma if `_i != 0`. Requires `_i` to be between 0 and 15.
 * @ingroup useful_macros
 */
#define AUI_PP_COMMA_IF(_i) AUI_PP_CAT(AUI_PP_INTERNAL_COMMA_IF_, _i)

/*
 * Internal details follow. Do not use any of these symbols outside of this file or we will break your code.
 */
#define AUI_PP_INTENRAL_EMPTY_TUPLE (, , , , , , , , , , , , , , , )
#define AUI_PP_INTERNAL_CAT(_1, _2) _1##_2
#define AUI_PP_INTERNAL_STRINGIZE(...) #__VA_ARGS__
#define AUI_PP_INTERNAL_CAT_5(_1, _2, _3, _4, _5) _1##_2##_3##_4##_5
#define AUI_PP_INTERNAL_IS_EMPTY(_1, _2, _3, _4)                             \
  AUI_PP_HAS_COMMA(AUI_PP_INTERNAL_CAT_5(AUI_PP_INTERNAL_IS_EMPTY_CASE_, \
                                             _1, _2, _3, _4))
#define AUI_PP_INTERNAL_IS_EMPTY_CASE_0001 ,
#define AUI_PP_INTERNAL_IF_1(_Then, _Else) _Then
#define AUI_PP_INTERNAL_IF_0(_Then, _Else) _Else

// Because of MSVC treating a token with a comma in it as a single token when
// passed to another macro, we need to force it to evaluate it as multiple
// tokens. We do that by using a "IDENTITY(MACRO PARENTHESIZED_ARGS)" macro. We
// define one per possible macro that relies on this behavior. Note "_Args" must
// be parenthesized.
#define AUI_PP_INTERNAL_INTERNAL_16TH(_1, _2, _3, _4, _5, _6, _7, _8, _9, \
                                        _10, _11, _12, _13, _14, _15, _16,  \
                                        ...)                                \
  _16
#define AUI_PP_INTERNAL_16TH(_Args) \
  AUI_PP_IDENTITY(AUI_PP_INTERNAL_INTERNAL_16TH _Args)
#define AUI_PP_INTERNAL_INTERNAL_HEAD(_1, ...) _1
#define AUI_PP_INTERNAL_HEAD(_Args) \
  AUI_PP_IDENTITY(AUI_PP_INTERNAL_INTERNAL_HEAD _Args)
#define AUI_PP_INTERNAL_INTERNAL_TAIL(_1, ...) __VA_ARGS__
#define AUI_PP_INTERNAL_TAIL(_Args) \
  AUI_PP_IDENTITY(AUI_PP_INTERNAL_INTERNAL_TAIL _Args)

#define AUI_PP_INTERNAL_IBP_IS_VARIADIC_C(...) 1 _
#define AUI_PP_INTERNAL_IBP_IS_VARIADIC_R_1 1,
#define AUI_PP_INTERNAL_IBP_IS_VARIADIC_R_AUI_PP_INTERNAL_IBP_IS_VARIADIC_C \
  0,
#define AUI_PP_INTERNAL_REMOVE_PARENS(...) __VA_ARGS__
#define AUI_PP_INTERNAL_INC_0 1
#define AUI_PP_INTERNAL_INC_1 2
#define AUI_PP_INTERNAL_INC_2 3
#define AUI_PP_INTERNAL_INC_3 4
#define AUI_PP_INTERNAL_INC_4 5
#define AUI_PP_INTERNAL_INC_5 6
#define AUI_PP_INTERNAL_INC_6 7
#define AUI_PP_INTERNAL_INC_7 8
#define AUI_PP_INTERNAL_INC_8 9
#define AUI_PP_INTERNAL_INC_9 10
#define AUI_PP_INTERNAL_INC_10 11
#define AUI_PP_INTERNAL_INC_11 12
#define AUI_PP_INTERNAL_INC_12 13
#define AUI_PP_INTERNAL_INC_13 14
#define AUI_PP_INTERNAL_INC_14 15
#define AUI_PP_INTERNAL_INC_15 16
#define AUI_PP_INTERNAL_COMMA_IF_0
#define AUI_PP_INTERNAL_COMMA_IF_1 ,
#define AUI_PP_INTERNAL_COMMA_IF_2 ,
#define AUI_PP_INTERNAL_COMMA_IF_3 ,
#define AUI_PP_INTERNAL_COMMA_IF_4 ,
#define AUI_PP_INTERNAL_COMMA_IF_5 ,
#define AUI_PP_INTERNAL_COMMA_IF_6 ,
#define AUI_PP_INTERNAL_COMMA_IF_7 ,
#define AUI_PP_INTERNAL_COMMA_IF_8 ,
#define AUI_PP_INTERNAL_COMMA_IF_9 ,
#define AUI_PP_INTERNAL_COMMA_IF_10 ,
#define AUI_PP_INTERNAL_COMMA_IF_11 ,
#define AUI_PP_INTERNAL_COMMA_IF_12 ,
#define AUI_PP_INTERNAL_COMMA_IF_13 ,
#define AUI_PP_INTERNAL_COMMA_IF_14 ,
#define AUI_PP_INTERNAL_COMMA_IF_15 ,
#define AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, _element) \
  _Macro(_i, _Data, _element)
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_0(_i, _Macro, _Data, _Tuple)
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_1(_i, _Macro, _Data, _Tuple) \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple)
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_2(_i, _Macro, _Data, _Tuple)    \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_1(AUI_PP_INC(_i), _Macro, _Data,    \
                                    (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_3(_i, _Macro, _Data, _Tuple)    \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_2(AUI_PP_INC(_i), _Macro, _Data,    \
                                    (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_4(_i, _Macro, _Data, _Tuple)    \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_3(AUI_PP_INC(_i), _Macro, _Data,    \
                                    (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_5(_i, _Macro, _Data, _Tuple)    \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_4(AUI_PP_INC(_i), _Macro, _Data,    \
                                    (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_6(_i, _Macro, _Data, _Tuple)    \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_5(AUI_PP_INC(_i), _Macro, _Data,    \
                                    (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_7(_i, _Macro, _Data, _Tuple)    \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_6(AUI_PP_INC(_i), _Macro, _Data,    \
                                    (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_8(_i, _Macro, _Data, _Tuple)    \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_7(AUI_PP_INC(_i), _Macro, _Data,    \
                                    (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_9(_i, _Macro, _Data, _Tuple)    \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_8(AUI_PP_INC(_i), _Macro, _Data,    \
                                    (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_10(_i, _Macro, _Data, _Tuple)   \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_9(AUI_PP_INC(_i), _Macro, _Data,    \
                                    (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_11(_i, _Macro, _Data, _Tuple)   \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_10(AUI_PP_INC(_i), _Macro, _Data,   \
                                     (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_12(_i, _Macro, _Data, _Tuple)   \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_11(AUI_PP_INC(_i), _Macro, _Data,   \
                                     (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_13(_i, _Macro, _Data, _Tuple)   \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_12(AUI_PP_INC(_i), _Macro, _Data,   \
                                     (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_14(_i, _Macro, _Data, _Tuple)   \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_13(AUI_PP_INC(_i), _Macro, _Data,   \
                                     (AUI_PP_TAIL _Tuple))
#define AUI_PP_INTERNAL_FOR_EACH_IMPL_15(_i, _Macro, _Data, _Tuple)   \
  AUI_PP_INTERNAL_CALL_MACRO(_Macro, _i, _Data, AUI_PP_HEAD _Tuple) \
  AUI_PP_INTERNAL_FOR_EACH_IMPL_14(AUI_PP_INC(_i), _Macro, _Data,   \
                                     (AUI_PP_TAIL _Tuple))

//NOLINTEND(modernize-*,cppcoreguidelines-macro-*)
