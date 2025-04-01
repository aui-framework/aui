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

#include <Windows.h>

/**
 * @brief RAII helper to access HGLOBAL data.
 */
class ScopedHGlobal {
public:
    explicit ScopedHGlobal(HGLOBAL blob) noexcept: mBlob(blob) {
        mAccessibleData = static_cast<char*>(GlobalLock(blob));
    }

    ~ScopedHGlobal() {
        GlobalUnlock(mBlob);
    }

    [[nodiscard]]
    char* data() const noexcept {
        return mAccessibleData;
    }

    [[nodiscard]]
    std::size_t size() const noexcept {
        return GlobalSize(mBlob);
    }

    operator AByteBufferView() const noexcept {
        return { data(), size() };
    }

private:
    HGLOBAL mBlob;
    char* mAccessibleData;
};