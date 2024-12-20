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

#include <concepts>
#include <utility>
#include <functional>
#include <type_traits>
#include "callables.h"

namespace aui {

    // the following concepts are partially copying stl functionality but android has no support for these
    template <class Derived, class Base>
    concept derived_from = std::is_base_of_v<Base, Derived> &&
                           std::is_convertible_v<const volatile Derived*, const volatile Base*>;

    /**
     * @brief Invokable concept.
     * @tparam Args argument types
     * @details
     * aui::invocable is replicated from STL's std::invocable.
     *
     * aui::invocable does not require return type. To specify excepted return type, use aui::callable instead.
     */
    template<typename F, typename... Args>
    concept invocable = requires(F&& f, Args&&... args) {
        { std::invoke(f, std::forward<Args>(args)...) };
    };

    template <class From, class To>
    concept convertible_to = std::is_convertible_v<From, To> && requires {
        static_cast<To>(std::declval<From>());
    };

    template < class T >
    concept destructible = std::is_nothrow_destructible_v<T>;

    template < class T, class... Args >
    concept constructible_from = destructible<T> && std::is_constructible_v<T, Args...>;

    template<typename _Tp>
    concept move_constructible = constructible_from<_Tp, _Tp> && convertible_to<_Tp, _Tp>;

    template< class T >
    concept copy_constructible = move_constructible<T> &&
                                 constructible_from<T, T&> &&
                                 convertible_to<T&, T> &&
                                 constructible_from<T, const T&> &&
                                 convertible_to<const T&, T> &&
                                 constructible_from<T, const T> &&
                                 convertible_to<const T, T>;

    namespace detail {
        template< class T, class U >
        concept SameHelper = std::is_same_v<T, U>;
    }

    template< class T, class U >
    concept same_as = detail::SameHelper<T, U> && detail::SameHelper<U, T>;



    template < class T, class U >
    concept common_reference_with = same_as<std::remove_reference_t<T>, std::remove_reference_t<T>> &&
                                    convertible_to<T, std::remove_reference_t<U>> &&
                                    convertible_to<U, std::remove_reference_t<T>>;

    template< class LHS, class RHS >
    concept assignable_from = std::is_lvalue_reference_v<LHS> &&
                              common_reference_with<const std::remove_reference_t<LHS>&,
                                                    const std::remove_reference_t<RHS>&> &&
                              requires(LHS lhs, RHS&& rhs) {
        { lhs = std::forward<RHS>(rhs) } -> std::same_as<LHS>;
    };
    template< class T >
    concept swappable =
    requires(T& a, T& b) {
        std::swap(a, b);
    };


    template < class T >
    concept movable = std::is_object_v<T> &&
                      move_constructible<T> &&
                      assignable_from<T&, T> &&
                      swappable<T>;

    template <class T>
    concept copyable = copy_constructible<T> &&
                       movable<T> &&
                       assignable_from<T&, T&> &&
                       assignable_from<T&, const T&> &&
                       assignable_from<T&, const T>;

    template<class T>
    concept default_initializable = constructible_from<T> &&
                                    requires { T{}; } &&
                                    requires { ::new T; };


    template <class T>
    concept semiregular = copyable<T> && default_initializable<T>;

    template <class T>
    concept regular = semiregular<T>;

    template<class I>
    concept incrementable = regular<I> && requires(I i) {
        { i++ } -> std::same_as<I>;
    };


    template<typename F, typename... Args>
    concept predicate = requires(F&& f, Args&&... args) {
        { f(std::forward<Args>(args)...) } -> same_as<bool>;
    };

    // aui concepts

    template<typename F, typename From, typename To>
    concept mapper = requires(F&& f, From&& from) {
        { std::invoke(f, std::forward<From>(from)) } -> aui::convertible_to<To>;
    };

    template<typename F, typename ProducedObject>
    concept factory = requires(F&& f) {
        { std::invoke(f) } -> aui::convertible_to<ProducedObject>;
    };

    /**
     * @brief Concept shortcut to std::is_arithmetic_v
     */
    template<typename T>
    concept arithmetic = std::is_arithmetic_v<T>;

    template<typename T>
    concept unsigned_integral = std::is_unsigned_v<T>;
}