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

#include <algorithm>
#include <tuple>
#include <string_view>

namespace aui::reflect {
/**
 * @brief Pointer to member type (not value) introspection.
 * @ingroup reflection
 * @details
 * `aui::reflect::member` provides metaprogramming capabilities to inspect the properties of class members.
 * Depending on the actual kind of the member, different kinds of data available.
 *
 * @note
 * Generally, prefer to use @ref aui::reflect::member_v which is easier to use and gives more data.
 * Use @ref aui::reflect::member only if you have type only.
 *
 * ## Fields (member variables)
 * @snippet aui.core/src/AUI/Reflect/members.h fields
 *
 * ## Methods (member functions)
 * @snippet aui.core/src/AUI/Reflect/members.h methods
 * @note
 * Use @ref aui::tuple_visitor and @ref aui::parameter_pack to introspect arguments.
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
concept pointer_to_member = requires(T&&) { typename aui::reflect::member<T>::clazz; };


/**
 * @brief Pointer to member value (not type) introspection.
 * @ingroup reflection
 * @details
 * In comparison to @ref aui::reflect::member, aui::reflect::member_v inspects pointer-to-member value, instead of type.
 * Since value points to exact member instead of generalizing by type, this allows to introspect additional data such as
 * member name.
 *
 * @snippet aui.core/tests/ReflectTest.cpp member_v
 *
 * # Derived data from aui::reflect::member
 * aui::reflect::member_v derives members from @ref aui::reflect::member.
 *
 * @copydetails aui::reflect::member
 */


}   // namespace aui::reflect