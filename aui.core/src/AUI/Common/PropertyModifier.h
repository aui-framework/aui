// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
//
// SPDX-License-Identifier: MPL-2.0
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <AUI/Traits/concepts.h>

namespace aui {
/**
 * @brief Temporary transparent object that gains write access to underlying property's value, notifying about value
 * changes when destructed.
 * @ingroup property_system
 * @details
 * PropertyModifier is a result of `writeScope()` method of writeable properties. It gains tranparent writeable handle
 * to property's value, and calls `notify()` methos on property upon PropertyModifier destructor.
 *
 * This ensures that a write access to the property is committed and can be observed.
 */
template<typename Property>
class PropertyModifier {
public:
    using Underlying = typename Property::Underlying;
    PropertyModifier(Property& owner): mOwner(&owner) {}
    ~PropertyModifier() {
        if (mOwner == nullptr) {
            return;
        }
        mOwner->notify();
    }

    [[nodiscard]]
    const Underlying& value() const noexcept {
        return mOwner->value();
    }

    [[nodiscard]]
    Underlying& value() noexcept {
        return const_cast<Underlying&>(mOwner->value());
    }

    [[nodiscard]]
    const Underlying* operator->() const noexcept {
        return &value();
    }

    [[nodiscard]]
    Underlying* operator->() noexcept {
        return &value();
    }

private:
    Property* mOwner;
};
}

template<typename T>
inline decltype(auto) operator*(aui::PropertyModifier<T>&& t) {
    return t.value();
}

template<typename T>
inline decltype(auto) operator*(const aui::PropertyModifier<T>& t) {
    return t.value();
}

template<typename T, typename Rhs>
[[nodiscard]]
inline auto operator==(const aui::PropertyModifier<T>& lhs, Rhs&& rhs) {
    return *lhs == std::forward<Rhs>(rhs);
}

template<typename T, typename Rhs>
[[nodiscard]]
inline auto operator!=(const aui::PropertyModifier<T>& lhs, Rhs&& rhs) {
    return *lhs != std::forward<Rhs>(rhs);
}

template<AAnyProperty T, typename Rhs>
[[nodiscard]]
inline auto operator+(const aui::PropertyModifier<T>& lhs, Rhs&& rhs) {
    return *lhs + std::forward<Rhs>(rhs);
}

template<AAnyProperty T, typename Rhs>
[[nodiscard]]
inline auto operator-(const aui::PropertyModifier<T>& lhs, Rhs&& rhs) {
    return *lhs - std::forward<Rhs>(rhs);
}

template<AAnyProperty T, typename Rhs>
[[nodiscard]]
inline auto operator+=(aui::PropertyModifier<T>& lhs, Rhs&& rhs) {
    return *lhs += std::forward<Rhs>(rhs);
}

template<AAnyProperty T, typename Rhs>
[[nodiscard]]
inline auto operator-=(aui::PropertyModifier<T>& lhs, Rhs&& rhs) {
    return *lhs -= std::forward<Rhs>(rhs);
}
