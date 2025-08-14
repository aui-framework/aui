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
#include <AUI/Platform/detail/key_to_native.h>

/*
 * Mac virtual key code to SDL scancode mapping table
 * Sources:
 * - Inside Macintosh: Text <http://developer.apple.com/documentation/mac/Text/Text-571.html>
 * - Apple USB keyboard driver source
 * <http://darwinsource.opendarwin.org/10.4.6.ppc/IOHIDFamily-172.8/IOHIDFamily/Cosmo_USB2ADB.c>
 * - experimentation on various ADB and USB ISO keyboards and one ADB ANSI keyboard
 */

static constexpr AInput::Key fromNativeImpl(int w) {
    switch (w) {
        case 0:
            return AInput::A;
        case 1:
            return AInput::S;
        case 2:
            return AInput::D;
        case 3:
            return AInput::F;
        case 4:
            return AInput::H;
        case 5:
            return AInput::G;
        case 6:
            return AInput::Z;
        case 7:
            return AInput::X;
        case 8:
            return AInput::C;
        case 9:
            return AInput::V;
        case 11:
            return AInput::B;
        case 12:
            return AInput::Q;
        case 13:
            return AInput::W;
        case 14:
            return AInput::E;
        case 15:
            return AInput::R;
        case 16:
            return AInput::Y;
        case 17:
            return AInput::T;
        case 18:
            return AInput::NUM1;
        case 19:
            return AInput::NUM2;
        case 20:
            return AInput::NUM3;
        case 21:
            return AInput::NUM4;
        case 22:
            return AInput::NUM6;
        case 23:
            return AInput::NUM5;
        case 24:
            return AInput::EQUAL;
        case 25:
            return AInput::NUM9;
        case 26:
            return AInput::NUM7;
        case 27:
            return AInput::DASH;
        case 28:
            return AInput::NUM8;
        case 29:
            return AInput::NUM0;
        case 30:
            return AInput::RBRACKET;
        case 31:
            return AInput::O;
        case 32:
            return AInput::U;
        case 33:
            return AInput::LBRACKET;
        case 34:
            return AInput::I;
        case 35:
            return AInput::P;
        case 36:
            return AInput::RETURN;
        case 37:
            return AInput::L;
        case 38:
            return AInput::J;
        case 39:
            return AInput::QUOTE;
        case 40:
            return AInput::K;
        case 41:
            return AInput::SEMICOLON;
        case 42:
            return AInput::BACKSLASH;
        case 43:
            return AInput::COMMA;
        case 44:
            return AInput::SLASH;
        case 45:
            return AInput::N;
        case 46:
            return AInput::M;
        case 47:
            return AInput::PERIOD;
        case 48:
            return AInput::TAB;
        case 49:
            return AInput::SPACE;
        case 50:
            return AInput::TILDE;
        case 51:
            return AInput::BACKSPACE;
        case 52:
            return AInput::RETURN;
        case 53:
            return AInput::ESCAPE;
        case 54:
            return AInput::RSYSTEM;
        case 55:
            return AInput::LSYSTEM;
        case 56:
            return AInput::LSHIFT;
        case 57:
            return AInput::CAPSLOCK;
        case 58:
            return AInput::LALT;
        case 59:
            return AInput::LCONTROL;
        case 60:
            return AInput::RSHIFT;
        case 61:
            return AInput::RALT;
        case 62:
            return AInput::RCONTROL;
        case 63:
            return AInput::RSYSTEM;
        case 64:
            return AInput::F17;
        case 65:
            return AInput::NUMPAD_PERIOD;
        case 66:
            return AInput::UNKNOWN;
        case 67:
            return AInput::NUMPAD_MULTIPLY;
        case 68:
            return AInput::UNKNOWN;
        case 69:
            return AInput::NUMPAD_ADD;
        case 70:
            return AInput::UNKNOWN;
        case 71:
            return AInput::NUMLOCKCLEAR;
        case 72:
            return AInput::VOLUMEUP;
        case 73:
            return AInput::VOLUMEDOWN;
        case 74:
            return AInput::MUTE;
        case 75:
            return AInput::NUMPAD_DIVIDE;
        case 76:
            return AInput::NUMPAD_RETURN;
        case 77:
            return AInput::UNKNOWN;
        case 78:
            return AInput::NUMPAD_SUBTRACT;
        case 79:
            return AInput::F18;
        case 80:
            return AInput::F19;
        case 81:
            return AInput::NUMPAD_EQUAL;
        case 82:
            return AInput::NUMPAD_0;
        case 83:
            return AInput::NUMPAD_1;
        case 84:
            return AInput::NUMPAD_2;
        case 85:
            return AInput::NUMPAD_3;
        case 86:
            return AInput::NUMPAD_4;
        case 87:
            return AInput::NUMPAD_5;
        case 88:
            return AInput::NUMPAD_6;
        case 89:
            return AInput::NUMPAD_7;
        case 90:
            return AInput::UNKNOWN;
        case 91:
            return AInput::NUMPAD_8;
        case 92:
            return AInput::NUMPAD_9;
        case 93:
            return AInput::INTERNATIONAL3;
        case 94:
            return AInput::INTERNATIONAL1;
        case 95:
            return AInput::NUMPAD_COMMA;
        case 96:
            return AInput::F5;
        case 97:
            return AInput::F6;
        case 98:
            return AInput::F7;
        case 99:
            return AInput::F3;
        case 100:
            return AInput::F8;
        case 101:
            return AInput::F9;
        case 102:
            return AInput::LANG2;
        case 103:
            return AInput::F11;
        case 104:
            return AInput::LANG1;
        case 105:
            return AInput::PRINTSCREEN;
        case 106:
            return AInput::F16;
        case 107:
            return AInput::SCROLLLOCK;
        case 108:
            return AInput::UNKNOWN;
        case 109:
            return AInput::F10;
        case 110:
            return AInput::MENU;
        case 111:
            return AInput::F12;
        case 112:
            return AInput::UNKNOWN;
        case 113:
            return AInput::PAUSE;
        case 114:
            return AInput::INSERT;
        case 115:
            return AInput::HOME;
        case 116:
            return AInput::PAGEUP;
        case 117:
            return AInput::DEL;
        case 118:
            return AInput::F4;
        case 119:
            return AInput::END;
        case 120:
            return AInput::F2;
        case 121:
            return AInput::PAGEDOWN;
        case 122:
            return AInput::F1;
        case 123:
            return AInput::LEFT;
        case 124:
            return AInput::RIGHT;
        case 125:
            return AInput::DOWN;
        case 126:
            return AInput::UP;
        case 127:
            return AInput::POWER;
        default:
            return AInput::UNKNOWN;
    }
}

static constexpr auto KEY_TO_NATIVE = aui::detail::keyToNative(fromNativeImpl);

AInput::Key AInput::fromNative(int w) {
    return fromNativeImpl(w);
}

int AInput::toNative(AInput::Key w) {
    if (KEY_TO_NATIVE.size() <= w) {
        return w;
    }
    return KEY_TO_NATIVE[w];
}

bool gKeyStates[AInput::KEYCOUNT+1]{};

bool AInput::native::isKeyDown(AInput::Key k) {
    return gKeyStates[k];
}
