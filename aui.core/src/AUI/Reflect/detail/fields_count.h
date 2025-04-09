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

#include <cstdint>
#include <type_traits>
#include <limits>
#include "AUI/Traits/unsafe_declval.h"

namespace aui::reflect::detail {

// based on ideas found in Boost.PFR.
// credits: Antony Polukhin

/**
 * @brief Structure that can be converted to reference to anything.
 */
struct lref_constructing {
    std::size_t ignore;
    template <class Type>
    constexpr operator Type&() const&& noexcept {
        return aui::unsafe_declval<Type&>();
    }

    template <class Type>
    constexpr operator Type&() const& noexcept {
        return aui::unsafe_declval<Type&>();
    }
};

/**
 * @brief Structure that can be converted to rvalue reference to anything.
 */
struct rref_constructing {
    std::size_t ignore;
    template <class Type>
    /*constexpr*/
    operator Type() const&& noexcept {   // allows initialization of rvalue reference fields and move-only types
        return aui::unsafe_declval<Type>();
    }
};

/**
 * @brief Helper for SFINAE on fields count.
 */
template <
    class T, std::size_t... I, class /*Enable*/ = typename std::enable_if<std::is_copy_constructible<T>::value>::type>
constexpr auto enable_if_constructible(std::index_sequence<I...>) noexcept -> typename std::add_pointer<decltype(T {
  lref_constructing { I }... })>::type;

template <
    class T, std::size_t... I, class /*Enable*/ = typename std::enable_if<!std::is_copy_constructible<T>::value>::type>
constexpr auto enable_if_constructible(std::index_sequence<I...>) noexcept -> typename std::add_pointer<decltype(T {
  rref_constructing { I }... })>::type;

template <
    class T, std::size_t N,
    class /*Enable*/ = decltype(enable_if_constructible<T>(std::make_integer_sequence<std::size_t, N>()))>
using enable_if_constructible_t = std::size_t;

/**
 * @brief Greedy search.
 * @details
 * Templates instantiation depth is log(sizeof(T)), templates instantiation count is log(sizeof(T))*T in worst case.
 */
template <class T, std::size_t N>
constexpr auto detect_fields_count_greedy_remember(long) noexcept -> detail::enable_if_constructible_t<T, N> {
    return N;
}

template <class T, std::size_t N>
constexpr std::size_t detect_fields_count_greedy_remember(int) noexcept {
    return 0;
}

using multi_element_range = std::false_type;
using one_element_range = std::true_type;

template <std::size_t Begin, std::size_t Last>
using is_one_element_range = std::integral_constant<bool, Begin == Last>;

template <class T, std::size_t Begin, std::size_t Last>
constexpr std::size_t detect_fields_count_greedy(detail::one_element_range) noexcept {
    static_assert(
        Begin == Last,
        "====================> aui::reflect: internal logic error. Please report this issue to the github along with the "
        "structure you're reflecting.");
    return detail::detect_fields_count_greedy_remember<T, Begin>(1L);
}

template <class T, std::size_t Begin, std::size_t Last>
constexpr std::size_t detect_fields_count_greedy(detail::multi_element_range) noexcept {
    constexpr std::size_t middle = Begin + (Last - Begin) / 2;
    constexpr std::size_t fields_count_big_range =
        detail::detect_fields_count_greedy<T, middle + 1, Last>(detail::is_one_element_range<middle + 1, Last> {});

    constexpr std::size_t small_range_begin = (fields_count_big_range ? 0 : Begin);
    constexpr std::size_t small_range_last = (fields_count_big_range ? 0 : middle);
    constexpr std::size_t fields_count_small_range = detail::detect_fields_count_greedy<
        T, small_range_begin, small_range_last>(detail::is_one_element_range<small_range_begin, small_range_last> {});
    return fields_count_big_range ? fields_count_big_range : fields_count_small_range;
}

/**
 * @brief Non-greedy fields count search.
 * @details
 * Performs compile-time binary search, starting from the middle. The search is initiated by one of the
 * `detect_fields_count_dispatch` overloads. The search is performed recursively. Once a suitable argument count is
 * found, the recursion collapses, returning count.
 *
 * Templates instantiation depth is log(sizeof(T)), templates instantiation count is log(sizeof(T)).
 */
template <class T, std::size_t Begin, std::size_t Middle>
constexpr std::size_t detect_fields_count(detail::one_element_range, long) noexcept {
    static_assert(
        Begin == Middle,
        "====================> aui::reflect: internal logic error. Please report this issue to the github along with the "
        "structure you're reflecting.");
    return Begin;
}

template <class T, std::size_t Begin, std::size_t Middle>
constexpr std::size_t detect_fields_count(detail::multi_element_range, int) noexcept;

template <class T, std::size_t Begin, std::size_t Middle>
constexpr auto
detect_fields_count(detail::multi_element_range, long) noexcept -> detail::enable_if_constructible_t<T, Middle> {
    constexpr std::size_t next_v = Middle + (Middle - Begin + 1) / 2;
    return detail::detect_fields_count<T, Middle, next_v>(detail::is_one_element_range<Middle, next_v> {}, 1L);
}

template <class T, std::size_t Begin, std::size_t Middle>
constexpr std::size_t detect_fields_count(detail::multi_element_range, int) noexcept {
    constexpr std::size_t next_v = Begin + (Middle - Begin) / 2;
    return detail::detect_fields_count<T, Begin, next_v>(detail::is_one_element_range<Begin, next_v> {}, 1L);
}

/**
 * @brief Chooses between array, greedy and non-greedy search.
 * @details
 * The choose logic is made by substituting unused long/int arguments. Since this function is called with long
 * arguments, the first method is prioritized. We'll use the greedy method (the slowest) if others are not applicable.
 */
template <class T, std::size_t N>
constexpr auto detect_fields_count_dispatch(std::integral_constant<std::size_t, N>, long, long) noexcept ->
    typename std::enable_if<std::is_array<T>::value, std::size_t>::type {
    return sizeof(T) / sizeof(typename std::remove_all_extents<T>::type);
}

template <class T, std::size_t N>
constexpr auto
detect_fields_count_dispatch(std::integral_constant<std::size_t, N>, long, int) noexcept -> decltype(sizeof(T {})) {
    constexpr std::size_t middle = N / 2 + 1;
    return detail::detect_fields_count<T, 0, middle>(detail::multi_element_range {}, 1L);
}

template <class T, std::size_t N>
constexpr std::size_t detect_fields_count_dispatch(std::integral_constant<std::size_t, N>, int, int) noexcept {
    // Detects the maximum number of default-constructible members in struct type T.
    // This is necessary because T, being non-default aggregate initializable,
    // has at least one member that isn't default constructible.
    //
    // We must manually check each constructor up to N parameters and return
    // the largest one that doesn't result in a compile-time error.
    return detail::detect_fields_count_greedy<T, 0, N>(detail::multi_element_range {});
}

/**
 * @brief Detects aggregates with inheritance.
 */
template <class Derived, class U>
constexpr bool static_assert_non_inherited() noexcept {
    static_assert(
        !std::is_base_of<U, Derived>::value, "====================> aui::reflect: inherited types are not supported.");
    return true;
}

template <class Derived>
struct lref_base_asserting {
    template <class Type>
    constexpr operator Type&() const&&   // tweak for template_unconstrained.cpp like cases
        noexcept(detail::static_assert_non_inherited<Derived, Type>())   // force the computation of assert function
    {
        return aui::unsafe_declval<Type&>();
    }

    template <class Type>
    constexpr operator Type&() const&                                    // tweak for optional_chrono.cpp like cases
        noexcept(detail::static_assert_non_inherited<Derived, Type>())   // force the computation of assert function
    {
        return aui::unsafe_declval<Type&>();
    }
};

template <class Derived>
struct rref_base_asserting {
    template <class Type>
    /*constexpr*/ operator Type() const&&   // Allows initialization of rvalue reference fields and move-only types
        noexcept(detail::static_assert_non_inherited<Derived, Type>())   // force the computation of assert function
    {
        return aui::unsafe_declval<Type>();
    }
};

template <
    class T, std::size_t I0, std::size_t... I,
    class /*Enable*/ = typename std::enable_if<std::is_copy_constructible<T>::value>::type>
constexpr auto
assert_first_is_not_base(std::index_sequence<I0, I...>) noexcept -> typename std::add_pointer<decltype(T {
  lref_base_asserting<T> {}, lref_constructing { I }... })>::type {
    return nullptr;
}

template <
    class T, std::size_t I0, std::size_t... I,
    class /*Enable*/ = typename std::enable_if<!std::is_copy_constructible<T>::value>::type>
constexpr auto
assert_first_is_not_base(std::index_sequence<I0, I...>) noexcept -> typename std::add_pointer<decltype(T {
  rref_base_asserting<T> {}, rref_constructing { I }... })>::type {
    return nullptr;
}

template <class T>
constexpr void* assert_first_is_not_base(std::index_sequence<>) noexcept {
    return nullptr;
}

/**
 * @brief Counts the number of non-static data member in a given type.
 * @tparam Clazz The type to count fields in.
 * @details
 * `Clazz` must meet the following requirements:
 * - It must not be a reference type.
 * - It must have no virtual functions (i.e., it's not polymorphic).
 * - It must be an aggregate type (i.e., must have no user defined constructors).
 */
template <class Clazz>
constexpr std::size_t fields_count() noexcept {
    using type = std::remove_cv_t<Clazz>;

    static_assert(
        !std::is_reference<type>::value, "====================> aui::reflect: attempt to get fields count on a reference.");

    static_assert(
        !std::is_polymorphic<type>::value,
        "====================> aui::reflect: type must have no virtual function, because otherwise it is not aggregate "
        "initializable.");

#ifdef __cpp_lib_is_aggregate
    static_assert(
        std::is_aggregate<type>::value || std::is_scalar<type>::value,
        "====================> aui::reflect: type must be aggregate initializable.");
#endif

#if defined(_MSC_VER) && (_MSC_VER <= 1920)
    // workaround for msvc compilers. versions <= 1920 have a limit of max 1024 elements in template parameter pack
    constexpr std::size_t max_fields_count = (sizeof(type) * CHAR_BIT >= 1024 ? 1024 : sizeof(type) * CHAR_BIT);
#else
    constexpr std::size_t max_fields_count = (sizeof(type) * CHAR_BIT);   // we multiply by CHAR_BIT because the type
                                                                          // may have bitfields in T
#endif

    constexpr std::size_t result =
        detail::detect_fields_count_dispatch<type>(std::integral_constant<std::size_t, max_fields_count> {}, 1L, 1L);

    detail::assert_first_is_not_base<type>(std::make_integer_sequence<std::size_t, result>());

#ifndef __cpp_lib_is_aggregate
    static_assert(
        is_aggregate_initializable_n<type, result>::value,
        "====================> aui::reflect: types with user specified constructors (non-aggregate initializable types) "
        "are not supported.");
#endif

    static_assert(
        result != 0 || std::is_empty<type>::value || std::is_fundamental<type>::value || std::is_reference<type>::value,
        "====================> aui::reflect: if there's no other failed static asserts then something went wrong. Please "
        "report this issue to the github along with the structure you're reflecting.");

    return result;
}
}   // namespace aui::reflect::detail