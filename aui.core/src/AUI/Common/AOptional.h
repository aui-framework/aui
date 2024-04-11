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

#include <cstdint>
#include <cassert>
#include <utility>
#include <optional>
#include <stdexcept>
#include <AUI/Core.h>


namespace aui::impl::optional {
    API_AUI_CORE void throwException(const char* message);
}

/**
 * @brief Utility wrapper implementing the stack-allocated (fast) optional idiom.
 * @ingroup useful_templates
 */
template<typename T>
class AOptional {
public:
    constexpr AOptional() noexcept = default;
    constexpr AOptional(std::nullopt_t) noexcept {}


    template<typename U = T,
             std::enable_if_t<std::is_constructible_v<T, U> && std::is_convertible_v<U&&, T>, bool> = true>
    constexpr AOptional(U&& rhs) noexcept: mInitialized(true) {
        new (ptrUnsafe()) T(std::forward<U>(rhs));
    }

    template<typename U = T,
             std::enable_if_t<std::is_constructible_v<T, U> && !std::is_convertible_v<U&&, T>, bool> = true>
    explicit constexpr AOptional(U&& rhs) noexcept: mInitialized(true) {
        new (ptrUnsafe()) T(std::forward<U>(rhs));
    }

    constexpr AOptional(const AOptional& rhs) {
        operator=(rhs);
    }

    constexpr AOptional(AOptional&& rhs) noexcept {
        operator=(std::move(rhs));
    }

    template<typename U>
    constexpr AOptional(const AOptional<U>& rhs) {
        operator=(rhs);
    }

    template<typename U>
    constexpr AOptional(AOptional<U>&& rhs) noexcept {
        operator=(std::move(rhs));
    }

    ~AOptional() {
        if (mInitialized) ptrUnsafe()->~T();
    }


    [[nodiscard]]
    bool hasValue() const noexcept {
        return mInitialized;
    }

    constexpr explicit operator bool() const noexcept {
        return hasValue();
    }

    template<typename... Args>
    constexpr AOptional<T>& emplace(Args&&... args) {
        if (mInitialized) {
            ptrUnsafe()->~T();
        }
        new (ptrUnsafe()) T(std::forward<Args>(args)...);
        mInitialized = true;
        return *this;
    }

    constexpr AOptional<T>& operator=(std::nullopt_t) noexcept {
        if (mInitialized) {
            ptrUnsafe()->~T();
            mInitialized = false;
        }
        return *this;
    }

    template<typename U = T, typename std::enable_if_t<std::is_convertible_v<U&&, T>, bool> = true>
    constexpr AOptional<T>& operator=(U&& rhs) noexcept {
        if (mInitialized) {
            ptrUnsafe()->~T();
        }
        new (ptrUnsafe()) T(std::forward<U>(rhs));
        mInitialized = true;
        return *this;
    }

    constexpr AOptional<T>& operator=(const AOptional& rhs) noexcept {
        if (rhs) {
            operator=(rhs.value());
        }
        return *this;
    }

    constexpr AOptional<T>& operator=(AOptional&& rhs) noexcept {
        if (rhs) {
            operator=(std::move(rhs.value()));
            rhs.reset();
        }
        return *this;
    }

    template<typename U>
    constexpr AOptional<T>& operator=(const AOptional<U>& rhs) noexcept {
        if (rhs) {
            operator=(rhs.value());
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
        if (mInitialized) {
            ptrUnsafe()->~T();
        }
        new (ptrUnsafe()) T(std::move(rhs));
        mInitialized = true;
        return *this;
    }

    [[nodiscard]]
    T& value() noexcept {
        AUI_ASSERTX(mInitialized, "optional is empty");
        return reinterpret_cast<T&>(mStorage);
    }

    [[nodiscard]]
    const T& value() const noexcept {
        AUI_ASSERTX(mInitialized, "optional is empty");
        return reinterpret_cast<const T&>(mStorage);
    }

    [[nodiscard]]
    T* ptr() noexcept {
        return &value();
    }

    [[nodiscard]]
    const T* ptr() const noexcept {
        return &value();
    }

    [[nodiscard]]
    T* operator->() noexcept {
        return ptr();
    }

    [[nodiscard]]
    const T* operator->() const noexcept {
        return ptr();
    }

    [[nodiscard]]
    T& operator*() noexcept {
        return value();
    }

    [[nodiscard]]
    const T& operator*() const noexcept {
        return value();
    }

    void reset() noexcept {
        if (mInitialized) {
            ptrUnsafe()->~T();
            mInitialized = false;
        }
    }

    /**
     * @brief value or exception
     */
    T& valueOrException(const char* message = "empty optional") {
        if (mInitialized) {
            return value();
        }
        aui::impl::optional::throwException(message);
        throw std::logic_error("should not have reached here"); // silence "not all control paths return a value" warning
    }

    /**
     * @brief value or exception
     */
    const T& valueOrException(const char* message = "empty optional") const {
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
    T valueOr(F&& alternative) const {
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
                alternative();
                AUI_ASSERTX(false, "should not have reached here");
                throw std::runtime_error("should not have reached here"); // stub exception
            } else {
                return alternative();
            }
        }
    }

    template<typename U>
    [[nodiscard]]
    bool operator==(const AOptional<U>& rhs) const noexcept {
        return (mInitialized == rhs.mInitialized) && (!mInitialized || value() == rhs.value());
    }

    template<typename U>
    [[nodiscard]]
    bool operator==(const U& rhs) const noexcept {
        return mInitialized && value() == rhs;
    }

    [[nodiscard]]
    bool operator==(const std::nullopt_t& rhs) const noexcept {
        return !mInitialized;
    }

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> mStorage;
    bool mInitialized = false;

    [[nodiscard]]
    T* ptrUnsafe() noexcept {
        return &valueUnsafe();
    }

    [[nodiscard]]
    T& valueUnsafe() noexcept {
        return reinterpret_cast<T&>(mStorage);
    }

};
