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

#include <algorithm>
#include <tuple>
#include <string_view>

#ifndef AUI_REFLECT_FIELD_NAMES_ENABLED
#if (__cplusplus >= 202002L) || (defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L))
#if (defined(__cpp_nontype_template_args) && __cpp_nontype_template_args >= 201911) || \
    (defined(__clang_major__) && __clang_major__ >= 12)
#define AUI_REFLECT_FIELD_NAMES_ENABLED 1
#else
#define AUI_REFLECT_FIELD_NAMES_ENABLED 0
#endif
#else
#define AUI_REFLECT_FIELD_NAMES_ENABLED 0
#endif
#endif

namespace aui::reflect {
/**
 * @brief Pointer to member type (not value) introspection.
 * @ingroup reflection
 * @details
 * `aui::reflect::member` provides metaprogramming capabilities to inspect the properties of class members.
 * Depending on the actual kind of the member, different kinds of data available.
 *
 * ## Fields (member variables)
 *
 * <!-- aui:snippet aui.core/src/AUI/Reflect/members.h fields -->
 *
 * ## Methods (member functions)
 *
 * <!-- aui:snippet aui.core/src/AUI/Reflect/members.h methods -->
 *
 * Use [aui::tuple_visitor] and [aui::parameter_pack] to introspect arguments.
 */
template <typename Type>
struct member;

/**
 * @internal
 */
/// [fields]
template <typename Type, typename Clazz>
struct member<Type(Clazz::*)> {
    using type = Type;     // field type
    using clazz = Clazz;   // class type that owns the field
};
/// [fields]

/**
 * @internal
 */
template <typename Type, typename Clazz, typename... Args>
struct member<Type (Clazz::*)(Args...) const> {
    using return_t = Type;
    using clazz = Clazz;

    using args = std::tuple<Args...>;

    static constexpr bool is_const = true;
    static constexpr bool is_noexcept = false;
};

/**
 * @internal
 */
/// [methods]
template <typename Type, typename Clazz, typename... Args>
struct member<Type (Clazz::*)(Args...)> {
    using return_t = Type;   // function return type
    using clazz = Clazz;     // class type that owns the fields

    using args = std::tuple<Args...>;   // function args (see note)

    static constexpr bool is_const = false;      // whether is const
    static constexpr bool is_noexcept = false;   // whether is noexcept
};
/// [methods]

/**
 * @internal
 */
template <typename Type, typename Clazz, typename... Args>
struct member<Type (Clazz::*)(Args...) noexcept> {
    using return_t = Type;
    using clazz = Clazz;

    using args = std::tuple<Args...>;

    static constexpr bool is_const = false;
    static constexpr bool is_noexcept = true;
};

/**
 * @internal
 */
template <typename Type, typename Clazz, typename... Args>
struct member<Type (Clazz::*)(Args...) const noexcept> {
    using return_t = Type;
    using clazz = Clazz;

    using args = std::tuple<Args...>;

    static constexpr bool is_const = true;
    static constexpr bool is_noexcept = true;
};

/**
 * @brief Concept of a pointer-to-member.
 * @ingroup reflection
 */
template <typename T>
concept pointer_to_member = requires(T &&) { typename aui::reflect::member<T>::clazz; };
}   // namespace aui::reflect