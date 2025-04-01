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

//
// Created by alex2 on 5/15/2021.
//


#pragma once


#include <AUI/Platform/AInput.h>
#include <AUI/Common/AVector.h>

class AShortcut {
private:
    AVector<AInput::Key> mKeys;
    friend inline AShortcut operator+(AShortcut k1, const AInput::Key& k2);

public:
    AShortcut() = default;
    AShortcut(const AVector<AInput::Key>& keys) : mKeys(keys) {}
    AShortcut(AInput::Key keys) : mKeys({keys}) {}
    AShortcut(AVector<AInput::Key>&& keys) noexcept : mKeys(std::move(keys)) {}

    const AVector<AInput::Key>& getKeys() const {
        return mKeys;
    }

    operator AString() const {
        AString s;
        for (auto& k : mKeys) {
            if (!s.empty()) {
                s += "+";
            }
            s += AInput::getName(k);
        }
        return s;
    }
    bool empty() const {
        return mKeys.empty();
    }
};

inline AShortcut operator+(const AInput::Key& k1, const AInput::Key& k2) {
    return AShortcut({k1, k2});
}

inline AShortcut operator+(AShortcut k1, const AInput::Key& k2) {
    auto k = std::move(k1.mKeys);
    k << k2;
    return std::move(k);
}

