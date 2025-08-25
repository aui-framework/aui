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

#include <type_traits>
#include <cassert>
#include <variant>
#include <utility>
#include <functional>
#include <optional>
#include <glm/glm.hpp>
#include <AUI/Common/SharedPtrTypes.h>
#include <AUI/Common/AOptional.h>
#include <AUI/Thread/AMutex.h>
#include <AUI/Traits/concepts.h>

namespace aui {
namespace detail {
API_AUI_CORE void evaluationLoop();
}

/**
 * @brief Forbids copy of your class.
 *
 * ```cpp
 * class MyObject: public aui::noncopyable {
 * private:
 *     void* mResource;
 * }
 * ...
 * MyObject obj1;
 * MyObject obj2 = obj1; // error
 * MyObject obj3 = std::move(obj); // but this one is ok
 * ```
 */
struct noncopyable {
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

/**
 * Null-checking wrapper when usage of null is fatal.
 * @tparam T any pointer or pointer-like type
 */
template <typename T>
class assert_not_used_when_null {
private:
    T mValue;

public:
    assert_not_used_when_null(T value) noexcept : mValue(std::move(value)) {}

    template <typename AnyType>
    operator AnyType() noexcept {
        if constexpr (!std::is_same_v<AnyType, bool>) {
            AUI_ASSERTX(mValue != nullptr, "value is used when null");
        }
        return AnyType(mValue);
    }

    [[nodiscard]]
    T value() const noexcept {
        return mValue;
    }

    template <typename AnyType>
    bool operator==(const AnyType& v) const noexcept {
        return mValue == v;
    }

    template <typename AnyType>
    bool operator!=(const AnyType& v) const noexcept {
        return mValue != v;
    }
};

/**
 * @brief A contract that enforces non-nullable initialization at a later point in time.
 * @tparam T wrapped pointer-like type
 * @details
 * Same as aui::non_null but allows default ctor. This is particularly useful for scenarios where initial setup data is
 * unavailable at object creation time, but you require a guarantee that the value will be assigned before it is
 * accessed.
 *
 * If initialized with a value that equals to `nullptr`, issues a runtime assertion failure.
 *
 * If initialized with `nullptr` itself, throws a compile-time error.
 */
template <typename T>
struct non_null_lateinit {
private:
    static_assert(!std::is_reference_v<T>, "====================> aui::non_null: reference type is not allowed");
    void checkForNull() const { AUI_ASSERTX(value != nullptr, "this value couldn't be nullptr"); }

public:
    T value;
    non_null_lateinit() {}

    template <convertible_to<T> F>
    non_null_lateinit(F&& value) : value(std::forward<F>(value)) {
        static_assert(
            !std::is_same_v<std::decay_t<F>, std::nullptr_t>, "====================> aui::non_null: this value couldn't be nullptr");
        checkForNull();
    }

    operator const T&() const noexcept {
        checkForNull();
        return value;
    }

    operator T&() noexcept {
        checkForNull();
        return value;
    }

    auto operator->() const {
        checkForNull();
        return &*value;
    }
};

/**
 * @brief A contract that enforces non-nullable initialization.
 * @tparam T wrapped pointer-like type
 * @details
 *
 * If initialized with a value that equals to `nullptr`, issues a runtime assertion failure.
 *
 * If initialized with `nullptr` itself, throws a compile-time error.
 *
 * ```cpp
 * void render(aui::non_null<_<AView>> view) {
 *     view->render();
 * }
 *
 * render(someView); // ok
 * someView = nullptr;
 * render(someView); // assertion failure in runtime
 * render(nullptr); // compile-time error
 * ```
 */
template <typename T>
struct non_null : non_null_lateinit<T> {
    template <convertible_to<T> F>
    non_null(F&& value) : non_null_lateinit<T>(std::forward<F>(value)) {}

    non_null() = delete;   // disallow the default ctor inherited from lateinit.
};

/**
 * @brief A funcntion contract that does not allow escaping, allowing to accept lvalue ref, rvalue ref, shared_ptr and
 * etc without overhead
 *
 * @details
 * Promises that the contained object wouldn't be copied/moved/referenced outside the function where the
 * no_escape came to; thus does not take responsibility of deleting the object. This allows to accept any kind of
 * lifetimes: lvalue and rvalue references, pointers, unique_ptr and shared_ptr without ref counter modification.
 *
 * Intended to use in function arguments.
 *
 * Accepts lvalue ref, rvalue ref, ptr and shared_ptr. Does not accepts null.
 *
 * @tparam T undecorated type
 */
template <typename T>
struct no_escape {
    static_assert(
        !std::is_reference<T>::value,
        "====================> AUI: attempt to use aui::no_escape with reference type. Please use undecorated type "
        "(without reference)");
    static_assert(
        !std::is_pointer_v<T>,
        "====================> AUI: attempt to use aui::no_escape with pointer type. Please use undecorated type "
        "(without pointer)");

private:
    T* value;

public:
    no_escape(T& value) : value(&value) { AUI_ASSERTX(no_escape::value != nullptr, "the argument could not be null"); }
    // referring to a temporary value; no_escape should never be used anything else than as argument
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
    no_escape(T&& value) : value(&value) { AUI_ASSERTX(no_escape::value != nullptr, "the argument could not be null"); }
    no_escape(T* value) : value(value) { AUI_ASSERTX(no_escape::value != nullptr, "the argument could not be null"); }

    no_escape(const _<T>& value) : value(&*value) {
        AUI_ASSERTX(no_escape::value != nullptr, "the argument could not be null");
    }
    no_escape(const _unique<T>& value) : value(&*value) {
        AUI_ASSERTX(no_escape::value != nullptr, "the argument could not be null");
    }

    template <
        typename DerivedFromT,
        std::enable_if_t<std::is_base_of_v<T, DerivedFromT> && !std::is_same_v<DerivedFromT, T>, bool> = true>
    no_escape(const _<DerivedFromT>& value) : value(&*value) {
        AUI_ASSERTX(no_escape::value != nullptr, "the argument could not be null");
    }

    template <
        typename DerivedFromT,
        std::enable_if_t<std::is_base_of_v<T, DerivedFromT> && !std::is_same_v<DerivedFromT, T>, bool> = true>
    no_escape(const _unique<DerivedFromT>& value) : value(&*value) {
        AUI_ASSERTX(no_escape::value != nullptr, "the argument could not be null");
    }

    [[nodiscard]]
    T* ptr() const noexcept {
        return value;
    }

    T* operator->() const noexcept { return value; }

    T& operator*() const noexcept { return *value; }
};

/**
 * @brief A value that initializes when accessed for the first time.
 * @tparam T
 */
template <typename T = void>
struct lazy {
private:
    struct EvaluationLoopTrap {};
    mutable std::variant<std::nullopt_t, T, EvaluationLoopTrap> value = std::nullopt;
    std::function<T()> initializer;

public:
    template <typename Factory, std::enable_if_t<std::is_invocable_r_v<T, Factory>, bool> = true>
    lazy(Factory&& initializer) noexcept : initializer(std::forward<Factory>(initializer)) {}

    lazy(const lazy<T>& other) noexcept : value(other.value), initializer(other.initializer) {}
    lazy(lazy<T>&& other) noexcept : value(std::move(other.value)), initializer(std::move(other.initializer)) {}

    T& get() {
        if (std::holds_alternative<T>(value)) {
            return std::get<T>(value);
        }
        if (std::holds_alternative<std::nullopt_t>(value)) {
            setEvaluationLoopTrap();
            try {
                value = initializer();
            } catch (...) {
                value = std::nullopt;
                throw;
            }
            return std::get<T>(value);
        }
        detail::evaluationLoop();
        throw;
    }

    void setEvaluationLoopTrap() { value = EvaluationLoopTrap {}; }

    const T& get() const { return const_cast<lazy<T>*>(this)->get(); }

    operator T&() { return get(); }
    operator const T&() const { return get(); }

    T& operator*() { return get(); }
    const T& operator*() const { return get(); }

    T* operator->() { return &get(); }
    T const* operator->() const { return &get(); }

    lazy<T>& operator=(T&& t) {
        value = std::move(t);
        return *this;
    }
    lazy<T>& operator=(const T& t) {
        value = t;
        return *this;
    }

    void reset() { value = std::nullopt; }

    explicit operator bool() const noexcept
        requires requires { !aui::same_as<T, bool>; }
    {
        return hasValue();
    }

    [[nodiscard]]
    bool hasValue() const noexcept {
        return std::holds_alternative<T>(value);
    }
};

/**
 * @brief A value that initializes when accessed for the first time.
 * @tparam T
 */
template <>
struct lazy<void> {
private:
    mutable bool value = false;
    std::function<void()> initializer;

public:
    template <typename Factory, std::enable_if_t<std::is_invocable_r_v<void, Factory>, bool> = true>
    lazy(Factory&& initializer) noexcept : initializer(std::forward<Factory>(initializer)) {}

    lazy(const lazy<void>& other) noexcept : value(other.value), initializer(other.initializer) {}
    lazy(lazy<void>&& other) noexcept : value(other.value), initializer(std::move(other.initializer)) {}

    void get() {
        if (!value) {
            value = true;
            initializer();
        }
    }
    void get() const { return const_cast<lazy<void>*>(this)->get(); }

    void operator*() { return get(); }
    const void operator*() const { return get(); }

    void reset() { value = false; }

    [[nodiscard]]
    bool hasValue() const noexcept {
        return value;
    }
};

/**
 * @brief A value that initializes when accessed for the first time.
 * Unlike <code>aui::lazy</code>, internal logic of <code>aui::atomic_lazy</code> is threadsafe.
 * @tparam T
 */
template <typename T>
struct atomic_lazy {
private:
    mutable AMutex sync;
    mutable AOptional<T> value;
    std::function<T()> initializer;

public:
    template <typename Factory, std::enable_if_t<std::is_invocable_r_v<T, Factory>, bool> = true>
    atomic_lazy(Factory&& initializer) : initializer(std::forward<Factory>(initializer)) {}

    atomic_lazy(const atomic_lazy<T>& other) {
        std::unique_lock lock(other.sync);
        value = other.value;
        initializer = other.initializer;
    }
    atomic_lazy(atomic_lazy<T>&& other) noexcept {
        std::unique_lock lock(other.sync);
        value = std::move(other.value);
        initializer = std::move(other.initializer);
    }

    T& get() {
        if (!value) {
            std::unique_lock lock(sync);
            if (!value) {
                value = initializer();
            }
        }
        return *value;
    }
    const T& get() const { return const_cast<atomic_lazy<T>*>(this)->get(); }

    operator T&() { return get(); }
    operator const T&() const { return get(); }

    T& operator*() { return get(); }
    const T& operator*() const { return get(); }

    T* operator->() { return &get(); }
    T const* operator->() const { return &get(); }

    atomic_lazy<T>& operator=(T&& t) {
        std::unique_lock lock(sync);
        value = std::move(t);
        return *this;
    }

    atomic_lazy<T>& operator=(const T& t) {
        std::unique_lock lock(sync);
        value = t;
        return *this;
    }

    void reset() {
        std::unique_lock lock(sync);
        value.reset();
    }

    [[nodiscard]]
    bool hasValue() const noexcept {
        std::unique_lock lock(sync);
        return value.hasValue();
    }
};

namespace constraint {
/**
 * @brief Avoids copy of the wrapped value, pointing to a reference.
 * @tparam T
 * @details
 * The caller can be sure his value wouldn't be copied.
 */
template <typename T>
class avoid_copy {
private:
    T* value;

public:
    avoid_copy(T& value) : value(&value) {   // implicit initializer
    }
    operator T&() const {   // implicit conversion
        return *value;
    }

    T& operator*() const {   // std dereference
        return *value;
    }
    T* operator->() const {   // allow pointer-style calls
        return value;
    }
};

/**
 * @brief Wraps the object forbidding copy.
 * @tparam T
 * @details
 * The caller can be sure his value wouldn't be copied even if copy constructor allows to do so.
 */
template <typename T>
class move_only {
private:
    T value;

public:
    move_only(T&& rhs) : value(std::move(rhs)) {}
    move_only(move_only&& rhs) noexcept : value(std::move(rhs.value)) {}
    move_only(const move_only&) = delete;

    operator const T&() const { return value; }
    operator T&() { return value; }

    const T& operator*() const { return value; }
    const T* operator->() const { return &value; }
    T* operator->() { return &value; }
};
}   // namespace constraint

/**
 * @brief Clamps the possible values for a number to the specified range: `[min;max]`
 * @ingroup useful_templates
 * @tparam UnderlyingType any arithmetic type
 * @tparam min minimum possible value
 * @tparam max maximum possible value
 */
template <aui::arithmetic UnderlyingType, auto /* UnderlyingType*/ min, auto /* UnderlyingType*/ max>   // min and max
                                                                                                        // are defined
                                                                                                        // auto because
                                                                                                        // some static
                                                                                                        // analyzers
                                                                                                        // won't work
                                                                                                        // with float
                                                                                                        // value
                                                                                                        // template
                                                                                                        // arguments
    requires aui::convertible_to<decltype(min), UnderlyingType> && aui::convertible_to<decltype(max), UnderlyingType>
struct ranged_number {
public:
    static constexpr auto MIN = min;
    static constexpr auto MAX = max;

    ranged_number(UnderlyingType value)
      : value(glm::clamp(value, static_cast<UnderlyingType>(min), static_cast<UnderlyingType>(max))) {}
    ranged_number() : value(min) {}

    operator UnderlyingType() const {   // make it possible to work with ranged_number like with the underlying type
        return value;
    }

    auto operator<=>(const ranged_number&) const = default;

private:
    UnderlyingType value;
};

using float_within_0_1 = ranged_number<float, 0, 1>;
}   // namespace aui