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

    /**
     * @brief Human-readable shortcut. "Ctrl+C" style on most platforms; on macOS modifiers are written in the
     * conventional ⌃⌥⇧⌘ order followed by the key without separators (e.g. "⇧⌘Z").
     */
    operator AString() const {
#if AUI_PLATFORM_MACOS
        AString modifiers;
        AString rest;
        const auto rank = [](AInput::Key k) {
            switch (k) {
                case AInput::LCONTROL: case AInput::RCONTROL: return 0;
                case AInput::LALT: case AInput::RALT: return 1;
                case AInput::LSHIFT: case AInput::RSHIFT: return 2;
                case AInput::LSYSTEM: case AInput::RSYSTEM: return 3;
                default: return 4;
            }
        };
        for (int r = 0; r <= 4; ++r) {
            for (auto& k : mKeys) {
                if (rank(k) == r) {
                    (r == 4 ? rest : modifiers) += AInput::getName(k);
                }
            }
        }
        return modifiers + rest;
#else
        AString s;
        for (auto& k : mKeys) {
            if (!s.empty()) {
                s += "+";
            }
            s += AInput::getName(k);
        }
        return s;
#endif
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

