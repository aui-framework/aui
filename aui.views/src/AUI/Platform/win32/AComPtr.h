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


#include <cassert>

/**
 * @brief Takes care of AddRef and Release() reference counting mechanism of COM objects.
 * @tparam T
 */
template<typename T>
class AComPtr {
public:

    AComPtr() {
        mValue = nullptr;
    }

    ~AComPtr() {
        if (mValue) {
            mValue->Release();
        }
    }

    T** operator&() noexcept {
        AUI_ASSERTX(mValue == nullptr, "value already set");
        return &mValue;
    }

    AComPtr(T* value): mValue(value) {}
    AComPtr(const AComPtr<T>& rhs): mValue(rhs.mValue) {
        if (mValue) {
            mValue->AddRef();
        }
    }

    AComPtr(AComPtr<T>&& rhs): mValue(rhs.mValue) {
        rhs.mValue = nullptr;
    }

    [[nodiscard]]
    T* value() const noexcept {
        AUI_ASSERT(mValue != nullptr);
        return mValue;
    }

    [[nodiscard]]
    T* operator*() const noexcept {
        return value();
    }

    [[nodiscard]]
    T* operator->() const noexcept {
        return value();
    }


    [[nodiscard]]
    operator T*() const noexcept {
        return value();
    }

    [[nodiscard]]
    operator bool() const noexcept {
        return mValue != nullptr;
    }

private:
    T* mValue;
};


