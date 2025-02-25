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
concept pointer_to_member = requires(T &&) { typename aui::reflect::member<T>::clazz; };

namespace detail {
#if AUI_COMPILER_MSVC && AUI_REFLECT_FIELD_NAMES_ENABLED

/**
 * @brief External linkage wrapper, so T is not enforced to be externally linked.
 */
template <class T>
struct wrapper {
    const T value;
};

/**
 * @brief Link time assert. If linker fails to link with it, it means that fake_object used in run time.
 */
template <class T>
extern const wrapper<T> DO_NOT_USE_REFLECTION_WITH_LOCAL_TYPES;

/**
 * @brief For returning non-default constructible types.
 * @tparam T
 * @details
 * Neither std::declval nor unsafe_declval are suitable here.
 */
template <class T>
constexpr const T &fake_object() noexcept {
    return DO_NOT_USE_REFLECTION_WITH_LOCAL_TYPES<T>.value;
}

template <
    class UniqueKey,   // https://developercommunity.visualstudio.com/t/__FUNCSIG__-outputs-wrong-value-with-C/10458554
    auto M>
consteval std::string_view name_of_field_impl() noexcept {
    std::string_view s = __FUNCSIG__;
    s = s.substr(s.rfind("->value->"));
    s = s.substr(sizeof("->value->") - 1);
    s = s.substr(0, s.rfind(">(void)"));
    return s;
}

template <
    class UniqueKey,   // https://developercommunity.visualstudio.com/t/__FUNCSIG__-outputs-wrong-value-with-C/10458554
    auto M>
consteval std::string_view name_of_field_impl_method() noexcept {
    std::string_view s = __FUNCSIG__;
    s = s.substr(s.rfind(':') + 1);
    s = s.substr(0, s.find('('));
    return s;
}
#endif
}   // namespace detail

/**
 * @brief Pointer to member value (not type) introspection.
 * @ingroup reflection
 * @details
 * In comparison to @ref aui::reflect::member, aui::reflect::member_v inspects pointer-to-member value, instead of type.
 * Since value points to exact member instead of generalizing by type, this allows to introspect additional data such as
 * member name:
 *
 * @snippet aui.core/src/AUI/Reflect/members.h getName
 *
 * # Example
 *
 * @snippet aui.core/tests/ReflectTest.cpp member_v
 * @snippet aui.core/tests/ReflectTest.cpp member_v2
 *
 * # Derived data from aui::reflect::member
 * aui::reflect::member_v derives members from @ref aui::reflect::member.
 *
 * @copydetails aui::reflect::member
 */
template <auto M>
struct member_v : member<decltype(M)> {
#if AUI_REFLECT_FIELD_NAMES_ENABLED

private:
    static consteval std::string_view getName() {
#if AUI_COMPILER_MSVC
        if constexpr (requires { typename member_v::return_t; }) {
            return detail::name_of_field_impl_method<typename member_v::clazz, M>();
        } else {
            return detail::name_of_field_impl<
                typename member_v::clazz,
                std::addressof(std::addressof(detail::fake_object<typename member_v::clazz>())->*M)>();
        }
#elif AUI_COMPILER_CLANG
        std::string_view s = __PRETTY_FUNCTION__;
        {
            auto last = s.rfind(']');
            auto begin = s.rfind('&');
            s = s.substr(begin, last - begin);
        }
        if (auto c = s.rfind(':')) {
            s = s.substr(c + 1);
        }
        return s;
#else
        std::string_view s = __PRETTY_FUNCTION__;
        {
            auto last = s.rfind(';');
            auto begin = s.rfind('&');
            s = s.substr(begin, last - begin);
        }
        if (auto c = s.rfind(':')) {
            s = s.substr(c + 1);
        }
        return s;
#endif
    }

public:
    /// [getName]
    /*
     * @brief Field name.
     * @details
     * Compile-time `std::string_view` that holds name of the field.
     *
     * It's implemented via forbidden compiler-specific magic and requires your `class`/`struct` to be defined with
     * external linkage, i.e., please do not use function local types.
     */
    static constexpr std::string_view name = getName();
    /// [getName]
#endif
};

}   // namespace aui::reflect