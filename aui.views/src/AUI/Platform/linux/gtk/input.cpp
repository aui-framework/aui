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

#include "PlatformAbstractionGtk.h"
#include "AUI/Platform/detail/key_to_native.h"

static constexpr AInput::Key fromNativeImpl(unsigned int w) {
    switch (w) {
        case GDK_KEY_A: return AInput::Key::A;
        case GDK_KEY_B: return AInput::Key::B;
        case GDK_KEY_C: return AInput::Key::C;
        case GDK_KEY_D: return AInput::Key::D;
        case GDK_KEY_E: return AInput::Key::E;
        case GDK_KEY_F: return AInput::Key::F;
        case GDK_KEY_G: return AInput::Key::G;
        case GDK_KEY_H: return AInput::Key::H;
        case GDK_KEY_I: return AInput::Key::I;
        case GDK_KEY_J: return AInput::Key::J;
        case GDK_KEY_K: return AInput::Key::K;
        case GDK_KEY_L: return AInput::Key::L;
        case GDK_KEY_M: return AInput::Key::M;
        case GDK_KEY_N: return AInput::Key::N;
        case GDK_KEY_O: return AInput::Key::O;
        case GDK_KEY_P: return AInput::Key::P;
        case GDK_KEY_Q: return AInput::Key::Q;
        case GDK_KEY_R: return AInput::Key::R;
        case GDK_KEY_S: return AInput::Key::S;
        case GDK_KEY_T: return AInput::Key::T;
        case GDK_KEY_U: return AInput::Key::U;
        case GDK_KEY_V: return AInput::Key::V;
        case GDK_KEY_W: return AInput::Key::W;
        case GDK_KEY_X: return AInput::Key::X;
        case GDK_KEY_Y: return AInput::Key::Y;
        case GDK_KEY_Z: return AInput::Key::Z;
        case GDK_KEY_0: return AInput::Key::NUM0;
        case GDK_KEY_1: return AInput::Key::NUM1;
        case GDK_KEY_2: return AInput::Key::NUM2;
        case GDK_KEY_3: return AInput::Key::NUM3;
        case GDK_KEY_4: return AInput::Key::NUM4;
        case GDK_KEY_5: return AInput::Key::NUM5;
        case GDK_KEY_6: return AInput::Key::NUM6;
        case GDK_KEY_7: return AInput::Key::NUM7;
        case GDK_KEY_8: return AInput::Key::NUM8;
        case GDK_KEY_9: return AInput::Key::NUM9;
        case GDK_KEY_Escape: return AInput::Key::ESCAPE;
        case GDK_KEY_Control_L: return AInput::Key::LCONTROL;
        case GDK_KEY_Shift_L: return AInput::Key::LSHIFT;
        case GDK_KEY_Alt_L: return AInput::Key::LALT;
        case GDK_KEY_Meta_L: return AInput::Key::LSYSTEM;
        case GDK_KEY_Control_R: return AInput::Key::RCONTROL;
        case GDK_KEY_Shift_R: return AInput::Key::RSHIFT;
        case GDK_KEY_Alt_R: return AInput::Key::RALT;
        case GDK_KEY_Meta_R: return AInput::Key::RSYSTEM;
        case GDK_KEY_Menu: return AInput::Key::MENU;
        case GDK_KEY_bracketleft: return AInput::Key::LBRACKET;
        case GDK_KEY_bracketright: return AInput::Key::RBRACKET;
        case GDK_KEY_semicolon: return AInput::Key::SEMICOLON;
        case GDK_KEY_comma: return AInput::Key::COMMA;
        case GDK_KEY_period: return AInput::Key::PERIOD;
        case GDK_KEY_apostrophe: return AInput::Key::QUOTE;
        case GDK_KEY_slash: return AInput::Key::SLASH;
        case GDK_KEY_backslash: return AInput::Key::BACKSLASH;
        case GDK_KEY_grave: return AInput::Key::TILDE;
        case GDK_KEY_equal: return AInput::Key::EQUAL;
        case GDK_KEY_minus: return AInput::Key::DASH;
        case GDK_KEY_space: return AInput::Key::SPACE;
        case GDK_KEY_ISO_Enter: return AInput::Key::RETURN;
        case GDK_KEY_Return: return AInput::Key::BACKSPACE;
        case GDK_KEY_Tab: return AInput::Key::TAB;
        case GDK_KEY_Page_Up: return AInput::Key::PAGEUP;
        case GDK_KEY_Page_Down: return AInput::Key::PAGEDOWN;
        case GDK_KEY_End: return AInput::Key::END;
        case GDK_KEY_Home: return AInput::Key::HOME;
        case GDK_KEY_Insert: return AInput::Key::INSERT;
        case GDK_KEY_Delete: return AInput::Key::DEL;
        case GDK_KEY_plus: return AInput::Key::ADD;
        case GDK_KEY_multiply: return AInput::Key::MULTIPLY;
        case GDK_KEY_division: return AInput::Key::DIVIDE;
        case GDK_KEY_Left: return AInput::Key::LEFT;
        case GDK_KEY_Right: return AInput::Key::RIGHT;
        case GDK_KEY_Up: return AInput::Key::UP;
        case GDK_KEY_Down: return AInput::Key::DOWN;
        case GDK_KEY_F1: return AInput::Key::F1;
        case GDK_KEY_F2: return AInput::Key::F2;
        case GDK_KEY_F3: return AInput::Key::F3;
        case GDK_KEY_F4: return AInput::Key::F4;
        case GDK_KEY_F5: return AInput::Key::F5;
        case GDK_KEY_F6: return AInput::Key::F6;
        case GDK_KEY_F7: return AInput::Key::F7;
        case GDK_KEY_F8: return AInput::Key::F8;
        case GDK_KEY_F9: return AInput::Key::F9;
        case GDK_KEY_F10: return AInput::Key::F10;
        case GDK_KEY_F11: return AInput::Key::F11;
        case GDK_KEY_F12: return AInput::Key::F12;
        case GDK_KEY_F13: return AInput::Key::F13;
        case GDK_KEY_F14: return AInput::Key::F14;
        case GDK_KEY_F15: return AInput::Key::F15;
        case GDK_KEY_F16: return AInput::Key::F16;
        case GDK_KEY_F17: return AInput::Key::F17;
        case GDK_KEY_F18: return AInput::Key::F18;
        case GDK_KEY_F19: return AInput::Key::F19;
        case GDK_KEY_Pause: return AInput::Key::PAUSE;
        case GDK_KEY_Caps_Lock: return AInput::Key::CAPSLOCK;
        case GDK_KEY_Scroll_Lock: return AInput::Key::SCROLLLOCK;
        case GDK_KEY_Sys_Req: return AInput::Key::PRINTSCREEN;
        case GDK_KEY_KP_0: return AInput::Key::NUMPAD_0;
        case GDK_KEY_KP_1: return AInput::Key::NUMPAD_1;
        case GDK_KEY_KP_2: return AInput::Key::NUMPAD_2;
        case GDK_KEY_KP_3: return AInput::Key::NUMPAD_3;
        case GDK_KEY_KP_4: return AInput::Key::NUMPAD_4;
        case GDK_KEY_KP_5: return AInput::Key::NUMPAD_5;
        case GDK_KEY_KP_6: return AInput::Key::NUMPAD_6;
        case GDK_KEY_KP_7: return AInput::Key::NUMPAD_7;
        case GDK_KEY_KP_8: return AInput::Key::NUMPAD_8;
        case GDK_KEY_KP_9: return AInput::Key::NUMPAD_9;
        case GDK_KEY_KP_Add: return AInput::Key::NUMPAD_ADD;
        case GDK_KEY_KP_Subtract: return AInput::Key::NUMPAD_SUBTRACT;
        case GDK_KEY_KP_Multiply: return AInput::Key::NUMPAD_MULTIPLY;
        case GDK_KEY_KP_Divide: return AInput::Key::NUMPAD_DIVIDE;
        case GDK_KEY_KP_Decimal: return AInput::Key::NUMPAD_PERIOD;
        case GDK_KEY_KP_Equal: return AInput::Key::NUMPAD_EQUAL;
        case GDK_KEY_KP_Enter: return AInput::Key::NUMPAD_RETURN;
        case GDK_KEY_AudioRaiseVolume: return AInput::Key::VOLUMEUP;
        case GDK_KEY_AudioLowerVolume: return AInput::Key::VOLUMEDOWN;
        case GDK_KEY_AudioMute: return AInput::Key::MUTE;
        case GDK_KEY_PowerOff: return AInput::Key::POWER;
        default:
            return AInput::UNKNOWN;
    }
}

static constexpr auto KEY_TO_NATIVE = aui::detail::keyToNative(fromNativeImpl);

bool gKeyStates[AInput::KEYCOUNT+1]{};

AInput::Key PlatformAbstractionGtk::inputFromNative2(unsigned int key) {
    return fromNativeImpl(key);
}

AInput::Key PlatformAbstractionGtk::inputFromNative(int k) { return inputFromNative2(unsigned(k)); }
int PlatformAbstractionGtk::inputToNative(AInput::Key key) { return KEY_TO_NATIVE[key]; }

bool PlatformAbstractionGtk::inputIsKeyDown(AInput::Key k) {
    return gKeyStates[k];
}
