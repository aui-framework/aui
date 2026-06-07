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

#include <coroutine>
#include <exception>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>


/**
 * @brief A C++20 coroutine-based generator that yields values lazily.
 * @tparam T The type of values yielded by the generator.
 * @ingroup core
 *
 * AYieldGenerator provides a range-like interface to coroutine-based generators.
 * It supports iteration via standard C++ iterators and can be used in range-based
 * for loops.
 *
 * Example usage:
 * ```cpp
 * AYieldGenerator<int> generateNumbers() {
 *     for (int i = 0; i < 10; ++i) {
 *         co_yield i;
 *     }
 * }
 *
 * for (auto value : generateNumbers()) {
 *     std::cout << value << '\n';
 * }
 * ```
 */
template<typename T>
class AYieldGenerator;

namespace aui::impl {

/**
 * @brief Promise type for AYieldGenerator coroutines.
 * @tparam T The value type yielded by the generator.
 */
template<typename T>
class AYieldGeneratorPromise {
public:
    using ValueType = std::remove_reference_t<T>;
    using ReferenceType = std::conditional_t<std::is_reference_v<T>, T, T&>;
    using PointerType = ValueType*;

    AYieldGeneratorPromise() = default;

    auto get_return_object() noexcept -> AYieldGenerator<T>;
    auto getReturnObject() noexcept -> AYieldGenerator<T> { return get_return_object(); }

    auto initial_suspend() const { return std::suspend_always{}; }
    auto initialSuspend() const { return initial_suspend(); }

    auto final_suspend() const noexcept {
        return std::suspend_always{};
    }
    auto finalSuspend() const noexcept { return final_suspend(); }

    template<typename U = T, std::enable_if_t<!std::is_rvalue_reference<U>::value, int> = 0>
    auto yield_value(std::remove_reference_t<T>& value) noexcept {
        mValue = std::addressof(value);
        return std::suspend_always{};
    }

    template<typename U = T, std::enable_if_t<!std::is_rvalue_reference<U>::value, int> = 0>
    auto yieldValue(std::remove_reference_t<T>& value) noexcept {
        return yield_value(value);
    }

    auto yield_value(std::remove_reference_t<T>&& value) noexcept {
        mValue = std::addressof(value);
        return std::suspend_always{};
    }

    auto yieldValue(std::remove_reference_t<T>&& value) noexcept {
        return yield_value(value);
    }

    auto unhandled_exception() -> void {
        mException = std::current_exception();
    }

    auto unhandledException() -> void {
        return unhandled_exception();
    }

    auto return_void() noexcept -> void {}

    auto returnVoid() noexcept -> void {
        return return_void();
    }

    auto value() const noexcept -> ReferenceType {
        return static_cast<ReferenceType>(*mValue);
    }

    auto rethrowIfException() -> void {
        if (mException) {
            std::rethrow_exception(mException);
        }
    }

private:
    PointerType mValue{nullptr};
    std::exception_ptr mException;
};

/**
 * @brief Sentinel value for AYieldGenerator iteration end.
 */
struct AYieldGeneratorSentinel {};

/**
 * @brief Iterator for AYieldGenerator.
 * @tparam T The value type of the generator.
 */
template<typename T>
class AYieldGeneratorIterator {
    using CoroutineHandle = std::coroutine_handle<AYieldGeneratorPromise<T>>;

public:
    using IteratorCategory = std::input_iterator_tag;
    using DifferenceType = std::ptrdiff_t;
    using ValueType = typename AYieldGeneratorPromise<T>::ValueType;
    using ReferenceType = typename AYieldGeneratorPromise<T>::ReferenceType;
    using PointerType = typename AYieldGeneratorPromise<T>::PointerType;

    AYieldGeneratorIterator() noexcept {}

    explicit AYieldGeneratorIterator(CoroutineHandle coroutine) noexcept
        : mCoroutine(coroutine) {}

    friend auto operator==(const AYieldGeneratorIterator& it, AYieldGeneratorSentinel) noexcept -> bool {
        return it.mCoroutine == nullptr || it.mCoroutine.done();
    }

    friend auto operator!=(const AYieldGeneratorIterator& it, AYieldGeneratorSentinel s) noexcept -> bool {
        return !(it == s);
    }

    friend auto operator==(AYieldGeneratorSentinel s, const AYieldGeneratorIterator& it) noexcept -> bool {
        return it == s;
    }

    friend auto operator!=(AYieldGeneratorSentinel s, const AYieldGeneratorIterator& it) noexcept -> bool {
        return it != s;
    }

    AYieldGeneratorIterator& operator++() {
        mCoroutine.resume();
        if (mCoroutine.done()) {
            mCoroutine.promise().rethrowIfException();
        }

        return *this;
    }

    auto operator++(int) -> void {
        (void)operator++();
    }

    ReferenceType operator*() const noexcept {
        return mCoroutine.promise().value();
    }

    PointerType operator->() const noexcept {
        return std::addressof(operator*());
    }

private:
    CoroutineHandle mCoroutine{nullptr};
};

} // namespace aui::impl

template<typename T>
class AYieldGenerator : public std::ranges::view_base {
public:
    using PromiseType = aui::impl::AYieldGeneratorPromise<T>;
    using Iterator = aui::impl::AYieldGeneratorIterator<T>;
    using Sentinel = aui::impl::AYieldGeneratorSentinel;

    AYieldGenerator() noexcept : mCoroutine(nullptr) {}

    AYieldGenerator(const AYieldGenerator&) = delete;

    AYieldGenerator(AYieldGenerator&& other) noexcept
        : mCoroutine(other.mCoroutine) {
        other.mCoroutine = nullptr;
    }

    auto operator=(const AYieldGenerator&) = delete;

    auto operator=(AYieldGenerator&& other) noexcept -> AYieldGenerator& {
        if (std::addressof(other) != this) {
            if (mCoroutine) {
                mCoroutine.destroy();
            }
            mCoroutine = std::exchange(other.mCoroutine, nullptr);
        }

        return *this;
    }

    ~AYieldGenerator() {
        if (mCoroutine) {
            mCoroutine.destroy();
        }
    }

    auto begin() -> Iterator {
        if (mCoroutine != nullptr) {
            mCoroutine.resume();
            if (mCoroutine.done()) {
                mCoroutine.promise().rethrowIfException();
            }
        }

        return Iterator{mCoroutine};
    }

    auto end() noexcept -> Sentinel {
        return Sentinel{};
    }

private:
    friend class aui::impl::AYieldGeneratorPromise<T>;

    explicit AYieldGenerator(std::coroutine_handle<PromiseType> coroutine) noexcept
        : mCoroutine(coroutine) {}

    std::coroutine_handle<PromiseType> mCoroutine;
};

namespace aui::impl {

template<typename T>
auto AYieldGeneratorPromise<T>::get_return_object() noexcept -> AYieldGenerator<T> {
    return AYieldGenerator<T>{std::coroutine_handle<AYieldGeneratorPromise<T>>::from_promise(*this)};
}

} // namespace aui::impl

// Specialization of coroutine_traits for lambda types that return AYieldGenerator
namespace std {

template<typename T, typename LambdaType>
struct coroutine_traits<AYieldGenerator<T>, LambdaType> {
    using promise_type = aui::impl::AYieldGeneratorPromise<T>;
};

} // namespace std
