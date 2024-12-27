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

#include <AUI/Common/ASignal.h>

namespace aui::detail::property {
template<typename Property> // can't use AAnyProperty here, as concept would depend on itself
inline auto makeAssignment(Property& property) {
    return ASlotDef {
        .boundObject = property.boundObject(),
        .invocable = [&property](std::decay_t<Property>::Underlying value) {
            property = std::move(value);
        },
    };
}
}

template <typename T>
struct AProperty: AObjectBase {
    using Underlying = T;

    T raw;
    emits<T> changed;

    AProperty()
        requires aui::default_initializable<T>
    = default;

    template <aui::convertible_to<T> U>
    AProperty(U&& value) noexcept : raw(std::forward<U>(value)) {}

    AObjectBase* boundObject() {
        return this;
    }

    template <aui::convertible_to<T> U>
    AProperty& operator=(U&& value) noexcept {
        static constexpr auto IS_COMPARABLE = requires { this->raw == value; };
        if constexpr (IS_COMPARABLE) {
            if (this->raw == value) [[unlikely]] {
                return *this;
            }
        }
        this->raw = std::forward<U>(value);
        emit changed(this->raw);
        return *this;
    }

    template <ASignalInvokable SignalInvokable>
    void operator^(SignalInvokable&& t) {
        t.invokeSignal(nullptr);
    }

    [[nodiscard]]
    bool operator==(const T& rhs) const noexcept {
        return raw == rhs;
    }

    [[nodiscard]]
    bool operator!=(const T& rhs) const noexcept {
        return raw != rhs;
    }

    [[nodiscard]] operator const T&() const noexcept { return raw; }

    [[nodiscard]]
    const T* operator->() const noexcept {
        return &raw;
    }

    [[nodiscard]]
    const T& operator*() const noexcept {
        return raw;
    }

    [[nodiscard]]
    T& operator*() noexcept {
        return raw;
    }

    /**
     * @brief Makes a callable that assigns value to this property.
     */
    [[nodiscard]]
    auto assignment() noexcept {
        return aui::detail::property::makeAssignment(*this);
    }
};
static_assert(AAnyProperty<AProperty<int>>, "AProperty does not conform AAnyProperty concept");

/**
 * @brief TODO
 * @details
 * # Performance considerations
 * APropertyDef [does not involve](https://godbolt.org/z/cYTrc3PPf ) extra runtime overhead between assignment and
 * getter/setter.
 */
template <typename M, typename Getter, typename Setter, typename SignalArg>
struct APropertyDef {
    const M* base;
    using Model = M;
    Getter get;
    Setter set;
    using GetterReturnT = decltype(std::invoke(get, base));
    using Underlying = std::decay_t<GetterReturnT>;
    const emits<SignalArg>& changed;

    template <aui::convertible_to<Underlying> U>
    APropertyDef& operator=(U&& u) {
        std::invoke(set, const_cast<Model*>(base), std::forward<U>(u));
        return *this;
    }

    [[nodiscard]]
    GetterReturnT value() const noexcept {
        return std::invoke(get, base);
    }

    [[nodiscard]]
    GetterReturnT operator*() const noexcept {
        return std::invoke(get, base);
    }

    [[nodiscard]]
    const Underlying* operator->() const noexcept {
        return &std::invoke(get, base);
    }

    [[nodiscard]]
    operator GetterReturnT() const noexcept { return std::invoke(get, base); }

    [[nodiscard]]
    M* boundObject() const {
        return const_cast<M*>(base);
    }

    /**
     * @brief Makes a callable that assigns value to this property.
     */
    [[nodiscard]]
    auto assignment() noexcept {
        return aui::detail::property::makeAssignment(*this);
    }
};

// binary operations for properties.
template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator+(const Lhs& lhs, Rhs&& rhs) requires requires { *lhs + rhs; } {
    return *lhs + std::forward<Rhs>(rhs);
}

template<AAnyProperty Lhs, typename Rhs>
[[nodiscard]]
inline auto operator-(const Lhs& lhs, Rhs&& rhs) requires requires { *lhs - rhs; } {
    return *lhs - std::forward<Rhs>(rhs);
}

// simple check above operators work.
static_assert(requires { AProperty<int>() + 1; });
