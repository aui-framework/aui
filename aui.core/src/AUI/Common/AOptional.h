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
#include <cassert>
#include <utility>
#include <optional>
#include <fstream>
#include <stdexcept>
#include "AUI/Traits/concepts.h"
#include <AUI/Core.h>


namespace aui::impl::optional {
    API_AUI_CORE void throwException(const char* message);
}

/**
 * @brief Utility wrapper implementing the stack-allocated (fast) optional idiom.
 * @ingroup useful_templates
 *
 * Key differences from `std::optional`:
 * - Additional helper methods like valueOrException() and map()
 * - Support for reference types via `AOptional<T&>`
 * - More flexible `valueOr()` that accepts both values and callbacks
 *
 * Example usage:
 * ```cpp
 * AOptional<int> opt; // Empty optional
 * opt = 42; // Assign value
 * 
 * if (opt) { // Check if has value
 *     std::cout << *opt; // Access value
 * }
 * 
 * // Using valueOr with default
 * int val = opt.valueOr(0);
 * 
 * // Using valueOr with callback
 * int val2 = opt.valueOr([]{ return calculateDefault(); });
 * 
 * // Map transformation
 * AOptional<std::string> strOpt = opt.map([](int x) { return std::to_string(x); });
 * ```
 */
template<typename T>
class AOptional {
public:
    constexpr AOptional() noexcept = default;
    constexpr AOptional(std::nullopt_t) noexcept {}

    /**
     * @brief Constructs an optional containing the given value.
     * @param rhs Value to store
     */
    template <
        typename U = T, std::enable_if_t<std::is_constructible_v<T, U> && std::is_convertible_v<U&&, T>, bool> = true>
    constexpr AOptional(U&& rhs) noexcept(std::is_nothrow_constructible_v<T, U&&>) : mInitialized(true) {
        new (ptrUnsafe()) T(std::forward<U>(rhs));
    }

    /**
     * @brief Constructs an optional containing the given value (explicit conversion).
     * @param rhs Value to store
     */
    template <
        typename U = T, std::enable_if_t<std::is_constructible_v<T, U> && !std::is_convertible_v<U&&, T>, bool> = true>
    explicit constexpr AOptional(U&& rhs) noexcept(std::is_nothrow_constructible_v<T, U&&>) : mInitialized(true) {
        new (ptrUnsafe()) T(std::forward<U>(rhs));
    }

    /**
     * @brief Copy constructor.
     */
    constexpr AOptional(const AOptional& rhs) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        if (rhs) {
            new (ptrUnsafe()) T(*rhs);
            mInitialized = true;
        }
    }

    /**
     * @brief Move constructor.
     */
    constexpr AOptional(AOptional&& rhs) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (rhs) {
            new (ptrUnsafe()) T(std::move(*rhs));
            mInitialized = true;
            rhs.reset();
        }
    }

    /**
     * @brief Converting copy constructor.
     */
    template <typename U>
    constexpr AOptional(const AOptional<U>& rhs) noexcept(std::is_nothrow_constructible_v<T, const U&>) {
        operator=(rhs);
    }

    /**
     * @brief Converting move constructor.
     */
    template <typename U>
    constexpr AOptional(AOptional<U>&& rhs) noexcept {
        operator=(std::move(rhs));
    }

    constexpr ~AOptional() {
        if (mInitialized) ptrUnsafe()->~T();
    }


    /**
     * @brief Checks if the optional contains a value.
     * @return true if contains value, false otherwise
     */
    [[nodiscard]]
    bool hasValue() const noexcept {
        return mInitialized;
    }

    /**
     * @brief Boolean conversion operator.
     * @return true if contains value, false otherwise
     */
    constexpr explicit operator bool() const noexcept { return hasValue(); }

    /**
     * @brief Constructs the contained value in-place.
     * @param args Arguments to forward to the constructor
     * @return Reference to this optional
     */
    template <typename... Args>
    constexpr AOptional<T>& emplace(Args&&... args) {
        reset();
        new (ptrUnsafe()) T(std::forward<Args>(args)...);
        mInitialized = true;
        return *this;
    }

    constexpr AOptional<T>& operator=(std::nullopt_t) noexcept {
        reset();
        return *this;
    }

    template<typename U = T, typename std::enable_if_t<std::is_convertible_v<U&&, T>, bool> = true>
    constexpr AOptional<T>& operator=(U&& rhs) noexcept(std::is_nothrow_constructible_v<T, U&&>) {
        reset();
        new (ptrUnsafe()) T(std::forward<U>(rhs));
        mInitialized = true;
        return *this;
    }

    constexpr AOptional<T>& operator=(const AOptional& rhs) noexcept(std::is_nothrow_copy_constructible_v<T>) {
        if (rhs) {
            operator=(rhs.value());
        } else {
            reset();
        }
        return *this;
    }

    constexpr AOptional<T>& operator=(AOptional&& rhs) noexcept(std::is_nothrow_move_constructible_v<T>) {
        if (rhs) {
            operator=(std::move(rhs.value()));
            rhs.reset();
        } else {
            reset();
        }
        return *this;
    }

    template<typename U>
    constexpr AOptional<T>& operator=(const AOptional<U>& rhs) noexcept(std::is_nothrow_constructible_v<T, const U&>) {
        if (rhs) {
            operator=(rhs.value());
        } else {
            reset();
        }
        return *this;
    }

    // we want to move the U value, not the whole optional
    // NOLINTBEGIN(cppcoreguidelines-rvalue-reference-param-not-moved)
    template<typename U>
    constexpr AOptional<T>& operator=(AOptional<U>&& rhs) noexcept {
        if (rhs) {
            operator=(std::move(rhs.value()));
            rhs.reset();
            return *this;
        } else {
            reset();
        }
        return *this;
    }
    //NOLINTEND(cppcoreguidelines-rvalue-reference-param-not-moved)

    template<typename U = T>
    constexpr AOptional<T>& operator=(T&& rhs) noexcept {
        reset();
        new (ptrUnsafe()) T(std::move(rhs));
        mInitialized = true;
        return *this;
    }

    /**
     * @brief Access the contained value.
     * @return Reference to the contained value
     * @throws Assertion error if empty
     */
    [[nodiscard]]
    constexpr T& value() {
        AUI_ASSERTX(mInitialized, "optional is empty");
        return reinterpret_cast<T&>(mStorage);
    }

    /**
     * @brief Access the contained value.
     * @return Const reference to the contained value
     * @throws Assertion error if empty
     */
    [[nodiscard]]
    constexpr const T& value() const {
        AUI_ASSERTX(mInitialized, "optional is empty");
        return reinterpret_cast<const T&>(mStorage);
    }

    /**
     * @brief Get pointer to contained value.
     * @return Pointer to contained value
     */
    [[nodiscard]]
    constexpr T* ptr() {
        return &value();
    }

    /**
     * @brief Get const pointer to contained value.
     * @return Const pointer to contained value
     */
    [[nodiscard]]
    constexpr const T* ptr() const {
        return &value();
    }

    /**
     * @brief Arrow operator for accessing members of contained value.
     * @return Pointer to contained value
     */
    [[nodiscard]]
    constexpr T* operator->() {
        return ptr();
    }

    /**
     * @brief Arrow operator for accessing members of contained value.
     * @return Const pointer to contained value
     */
    [[nodiscard]]
    constexpr const T* operator->() const {
        return ptr();
    }

    /**
     * @brief Dereference operator.
     * @return Reference to contained value
     */
    [[nodiscard]]
    constexpr T& operator*() {
        return value();
    }

    /**
     * @brief Dereference operator.
     * @return Const reference to contained value
     */
    [[nodiscard]]
    constexpr const T& operator*() const {
        return value();
    }

    constexpr void reset() noexcept {
        if (mInitialized) {
            ptrUnsafe()->~T();
            mInitialized = false;
        }
    }

    /**
     * @brief value or exception
     */
    constexpr T& valueOrException(const char* message = "empty optional") {
        if (mInitialized) {
            return value();
        }
        aui::impl::optional::throwException(message);
        throw std::logic_error("should not have reached here"); // silence "not all control paths return a value" warning
    }

    /**
     * @brief value or exception
     */
    constexpr const T& valueOrException(const char* message = "empty optional") const {
        if (mInitialized) {
            return value();
        }
        aui::impl::optional::throwException(message);
        throw std::logic_error("should not have reached here"); // silence "not all control paths return a value" warning
    }

    /**
     * @brief value or alternative (either value or callback)
     * @tparam F same as T or invocable returning T or invokable throwing a exception
     * @param alternative
     * @return
     */
    template<typename F>
    constexpr T valueOr(F&& alternative) const {
        if (mInitialized) {
            return value();
        }
        constexpr bool isSame = std::is_constructible_v<T, F>;
        constexpr bool isInvocable = std::is_invocable_v<F>;

        static_assert(isSame || isInvocable, "F is neither same as T nor invokable returning T nor invokable throwing a exception");

        if constexpr (isSame) {
            return std::forward<F>(alternative);
        } else if constexpr(isInvocable) {
            if constexpr (std::is_same_v<std::invoke_result_t<F>, void>) {
                // if F returns void, we expect that used throws an exception in the lambda.
                alternative();
                AUI_ASSERT_NO_CONDITION("valueOr lambda returns void; either return a value or throw an exception");
                // stub exception
                throw std::runtime_error("valueOr lambda returns void; either return a value or throw an exception");
            } else {
                return alternative();
            }
        }
    }

    template<typename U>
    [[nodiscard]]
    constexpr bool operator==(const AOptional<U>& rhs) const noexcept {
        return (mInitialized == rhs.mInitialized) && (!mInitialized || value() == rhs.value());
    }

    template<typename U>
    [[nodiscard]]
    constexpr bool operator==(const U& rhs) const noexcept {
        return mInitialized && value() == rhs;
    }

    [[nodiscard]]
    constexpr bool operator==(const std::nullopt_t& rhs) const noexcept {
        return !mInitialized;
    }

    /**
     * @brief If a value is present, apply the provided mapper function to it.
     * @param mapper mapper function to apply. The mapper is invoked as a STL projection (i.e., the mapper could be
     * lambda, pointer-to-member function or field).
     */
    template<aui::invocable<const T&> Mapper>
    [[nodiscard]]
    constexpr auto map(Mapper&& mapper) const -> AOptional<decltype(std::invoke(std::forward<Mapper>(mapper), value()))> {
        if (hasValue()) {
            return std::invoke(std::forward<Mapper>(mapper), value());
        }
        return std::nullopt;
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> mStorage{};
    bool mInitialized = false;

    [[nodiscard]]
    constexpr T* ptrUnsafe() noexcept {
        return &valueUnsafe();
    }

    [[nodiscard]]
    constexpr T& valueUnsafe() noexcept {
        return reinterpret_cast<T&>(mStorage);
    }
};

template<typename T>
class AOptional<T&> { // specialization for references
public:
    constexpr AOptional() noexcept = default;

    constexpr AOptional(std::nullopt_t) noexcept {}

    constexpr AOptional(T& ref) noexcept
        : mPtr(std::addressof(ref)) {}

    template<typename U,
             std::enable_if_t<std::is_convertible_v<U*, T*>, bool> = true>
    constexpr AOptional(const AOptional<U&>& rhs) noexcept
        : mPtr(rhs ? std::addressof(rhs.value()) : nullptr) {}

    constexpr AOptional& operator=(T& ref) noexcept {
        mPtr = std::addressof(ref);
        return *this;
    }

    constexpr AOptional& operator=(std::nullopt_t) noexcept {
        reset();
        return *this;
    }

    constexpr AOptional& operator=(const AOptional& rhs) noexcept {
        if (rhs) {
            mPtr = std::addressof(rhs.value());
        } else {
            reset();
        }
        return *this;
    }

    template<typename U,
             std::enable_if_t<std::is_convertible_v<U*, T*>, bool> = true>
    constexpr AOptional& operator=(const AOptional<U&>& rhs) noexcept {
        if (rhs) {
            mPtr = std::addressof(rhs.value());
        } else {
            reset();
        }
        return *this;
    }

    [[nodiscard]]
    constexpr bool hasValue() const noexcept {
        return mPtr != nullptr;
    }

    constexpr explicit operator bool() const noexcept {
        return hasValue();
    }

    constexpr AOptional& emplace(T& ref) noexcept {
        mPtr = std::addressof(ref);
        return *this;
    }

    template<typename... Args>
    AOptional& emplace(Args&&...) = delete;

    [[nodiscard]]
    constexpr T& value() {
        AUI_ASSERTX(mPtr != nullptr, "optional (reference) is empty");
        return *mPtr;
    }

    [[nodiscard]]
    constexpr const T& value() const {
        AUI_ASSERTX(mPtr != nullptr, "optional (reference) is empty");
        return *mPtr;
    }

    [[nodiscard]]
    constexpr T* ptr() {
        return mPtr;
    }

    [[nodiscard]]
    constexpr const T* ptr() const {
        return mPtr;
    }

    [[nodiscard]]
    constexpr T* operator->() {
        return ptr();
    }

    [[nodiscard]]
    constexpr const T* operator->() const {
        return ptr();
    }

    [[nodiscard]]
    constexpr T& operator*() {
        return value();
    }

    [[nodiscard]]
    constexpr const T& operator*() const {
        return value();
    }

    constexpr void reset() noexcept {
        mPtr = nullptr;
    }

    constexpr T& valueOrException(const char* message = "empty optional") {
        if (mPtr) return *mPtr;
        aui::impl::optional::throwException(message);
        throw std::logic_error("should not have reached here");
    }

    constexpr const T& valueOrException(const char* message = "empty optional") const {
        if (mPtr) return *mPtr;
        aui::impl::optional::throwException(message);
        throw std::logic_error("should not have reached here");
    }

    template<typename F>
    constexpr T& valueOr(F&& alternative) const {
        if (mPtr) return *mPtr;
        // For references, alternative must provide a T&
        if constexpr (std::is_invocable_v<F>) {
            return alternative();
        } else {
            static_assert(std::is_same_v<F, void>, "valueOr for AOptional<T&> requires invocable returning T&");
        }
    }

    template<typename U>
    [[nodiscard]]
    constexpr bool operator==(const AOptional<U&>& rhs) const noexcept {
        if (mPtr == nullptr || rhs.ptr() == nullptr)
            return mPtr == rhs.ptr();
        return *mPtr == *rhs.ptr();
    }

    template<typename U>
    [[nodiscard]]
    constexpr bool operator==(const U& rhs) const noexcept {
        return mPtr != nullptr && *mPtr == rhs;
    }

    [[nodiscard]]
    constexpr bool operator==(const std::nullopt_t&) const noexcept {
        return mPtr == nullptr;
    }

    template<aui::invocable<const T&> Mapper>
    [[nodiscard]]
    constexpr auto map(Mapper&& mapper) const
        -> AOptional<decltype(std::invoke(std::forward<Mapper>(mapper), value()))> {
        if (hasValue()) {
            return std::invoke(std::forward<Mapper>(mapper), value());
        }
        return std::nullopt;
    }

private:
    T* mPtr = nullptr;
};


template<typename T>
inline std::ostream& operator<<(std::ostream& o, const AOptional<T>& v) {
    if (!v) {
        o << "[empty]";
    } else {
        o << *v;
    }
    return o;
}

