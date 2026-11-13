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

#include <AUI/Platform/AInput.h>

#include <array>
#include <bitset>
#include <utility>

// The DOM KeyboardEvent.keyCode is the "native" key code. It is deprecated, yet every browser fills it and its values
// are layout independent for letters and digits (they match the Windows virtual-key codes).

namespace {
constexpr std::array KEY_CODES = std::to_array<std::pair<int, AInput::Key>>({
    { 8, AInput::BACKSPACE }, { 9, AInput::TAB }, { 13, AInput::RETURN }, { 16, AInput::LSHIFT },
    { 17, AInput::LCONTROL }, { 18, AInput::LALT }, { 19, AInput::PAUSE }, { 20, AInput::CAPSLOCK },
    { 27, AInput::ESCAPE }, { 32, AInput::SPACE }, { 33, AInput::PAGEUP }, { 34, AInput::PAGEDOWN },
    { 35, AInput::END }, { 36, AInput::HOME }, { 37, AInput::LEFT }, { 38, AInput::UP }, { 39, AInput::RIGHT },
    { 40, AInput::DOWN }, { 44, AInput::PRINTSCREEN }, { 45, AInput::INSERT }, { 46, AInput::DEL },
    { 91, AInput::LSYSTEM }, { 92, AInput::RSYSTEM }, { 93, AInput::MENU }, { 106, AInput::MULTIPLY },
    { 107, AInput::ADD }, { 109, AInput::SUBTRACT }, { 111, AInput::DIVIDE }, { 144, AInput::NUMLOCKCLEAR },
    { 145, AInput::SCROLLLOCK }, { 59, AInput::SEMICOLON }, { 186, AInput::SEMICOLON }, { 61, AInput::EQUAL },
    { 187, AInput::EQUAL }, { 188, AInput::COMMA }, { 173, AInput::DASH }, { 189, AInput::DASH },
    { 190, AInput::PERIOD }, { 191, AInput::SLASH }, { 192, AInput::TILDE }, { 219, AInput::LBRACKET },
    { 220, AInput::BACKSLASH }, { 221, AInput::RBRACKET }, { 222, AInput::QUOTE },
});

/// a contiguous run of keys: native codes `from..from+count` are `first..first+count`
struct KeyRange {
    int from;
    int count;
    AInput::Key first;
};
constexpr std::array KEY_RANGES = std::to_array<KeyRange>({
    { 'A', 26, AInput::A },
    { '0', 10, AInput::NUM0 },
    { 96, 10, AInput::NUMPAD_0 },
    { 112, 12, AInput::F1 },
});

std::bitset<AInput::KEYCOUNT>& pressed() {
    static std::bitset<AInput::KEYCOUNT> keys;
    return keys;
}
}   // namespace

AInput::Key AInput::fromNative(int k) {
    for (const auto& r : KEY_RANGES) {
        if (k >= r.from && k < r.from + r.count) {
            return AInput::Key(r.first + (k - r.from));
        }
    }
    for (const auto& [native, key] : KEY_CODES) {
        if (native == k) {
            return key;
        }
    }
    return UNKNOWN;
}

int AInput::toNative(Key key) {
    for (const auto& r : KEY_RANGES) {
        if (key >= r.first && key < r.first + r.count) {
            return r.from + (key - r.first);
        }
    }
    for (const auto& [native, k] : KEY_CODES) {
        if (k == key) {
            return native;
        }
    }
    return 0;
}

bool AInput::native::isKeyDown(Key k) {
    return k >= 0 && k < KEYCOUNT && pressed().test(k);
}

namespace aui::emscripten {
void setKeyDown(AInput::Key k, bool down) {
    if (k >= 0 && k < AInput::KEYCOUNT) {
        pressed().set(k, down);
    }
}
}   // namespace aui::emscripten
