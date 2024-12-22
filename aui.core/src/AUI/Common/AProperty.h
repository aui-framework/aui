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

template<typename T>
struct AProperty: aui::noncopyable {
    T raw;
    emits<T> changed;

    AProperty() requires aui::default_initializable<T> = default;

    template<aui::convertible_to<T> U>
    AProperty(U&& value) noexcept: raw(std::forward<U>(value)) {}

    template<aui::convertible_to<T> U>
    AProperty& operator=(U&& value) noexcept {
        if (this->raw == value) [[unlikely]] {
            return *this;
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

    [[nodiscard]]
    operator const T&() const noexcept {
        return raw;
    }

    [[nodiscard]]
    const T* operator->() const noexcept {
        return &raw;
    }
};
