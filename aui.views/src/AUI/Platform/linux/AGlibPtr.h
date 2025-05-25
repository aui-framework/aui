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

#include <cassert>
#include <glib-object.h>

/**
 * @brief Takes care of g_object_ref and g_object_unref() reference counting mechanism of Glib objects.
 * @ingroup core
 * @tparam T
 * @details
 * @exclusivefor{linux}
 */
template<typename T>
class AGlibPtr {
public:
    AGlibPtr() {
        mValue = nullptr;
    }

    ~AGlibPtr() {
        release();
    }

    void release() {
        if (!mValue) {
            return;
        }
        g_object_unref(mValue);
        mValue = nullptr;
    }

    T** operator&() noexcept {
        AUI_ASSERTX(mValue == nullptr, "value already set");
        return &mValue;
    }

    AGlibPtr(T* value): mValue(value) {}
    AGlibPtr(const AGlibPtr<T>& rhs): mValue(rhs.mValue) {
        if (mValue) {
            g_object_ref(mValue);
        }
    }

    AGlibPtr& operator=(T* value) {
        release();
        mValue = value;
        return *this;
    }

    AGlibPtr(AGlibPtr<T>&& rhs): mValue(rhs.mValue) {
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
