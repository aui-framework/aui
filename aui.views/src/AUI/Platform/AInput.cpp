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

#include <AUI/Platform/AInput.h>

static bool gKeyStateForTesting[AInput::KEYCOUNT] = {0};

AString AInput::getName(Key k)
{
    switch (k)
    {
        default:        return "Unknown Key";
        case AInput::A: return "A";
        case AInput::B: return "B";
        case AInput::C: return "C";
        case AInput::D: return "D";
        case AInput::E: return "E";
        case AInput::F: return "F";
        case AInput::G: return "G";
        case AInput::H: return "H";
        case AInput::I: return "I";
        case AInput::J: return "J";
        case AInput::K: return "K";
        case AInput::L: return "L";
        case AInput::M: return "M";
        case AInput::N: return "N";
        case AInput::O: return "O";
        case AInput::P: return "P";
        case AInput::Q: return "Q";
        case AInput::R: return "R";
        case AInput::S: return "S";
        case AInput::T: return "T";
        case AInput::U: return "U";
        case AInput::V: return "V";
        case AInput::W: return "W";
        case AInput::X: return "X";
        case AInput::Y: return "Y";
        case AInput::Z: return "Z";
        case AInput::NUM0: return "Num0";
        case AInput::NUM1: return "Num1";
        case AInput::NUM2: return "Num2";
        case AInput::NUM3: return "Num3";
        case AInput::NUM4: return "Num4";
        case AInput::NUM5: return "Num5";
        case AInput::NUM6: return "Num6";
        case AInput::NUM7: return "Num7";
        case AInput::NUM8: return "Num8";
        case AInput::NUM9: return "Num9";
        case AInput::ESCAPE: return "Escape";
        case AInput::LCONTROL: case AInput::RCONTROL: return "Ctrl";
        case AInput::LSHIFT: case AInput::RSHIFT: return "Shift";
        case AInput::LALT: case AInput::RALT: return "Alt";
        case AInput::LSYSTEM: case AInput::RSYSTEM: return "Win";
        case AInput::MENU: return "Menu";
        case AInput::LBRACKET: return "LBracket";
        case AInput::RBRACKET: return "RBracket";
        case AInput::SEMICOLON: return "SemiColon";
        case AInput::COMMA: return "Comma";
        case AInput::PERIOD: return "Period";
        case AInput::QUOTE: return "Quote";
        case AInput::SLASH: return "Slash";
        case AInput::BACKSLASH: return "BackSlash";
        case AInput::TILDE: return "Tilde";
        case AInput::EQUAL: return "Equal";
        case AInput::DASH: return "Dash";
        case AInput::SPACE: return "Space";
        case AInput::RETURN: return "Return";
        case AInput::BACKSPACE: return "BackSpace";
        case AInput::TAB: return "Tab";
        case AInput::PAGEUP: return "PageUp";
        case AInput::PAGEDOWN: return "PageDown";
        case AInput::END: return "End";
        case AInput::HOME: return "Home";
        case AInput::INSERT: return "Insert";
        case AInput::DEL: return "Delete";
        case AInput::ADD: return "Add";
        case AInput::SUBTRACT: return "Subtract";
        case AInput::MULTIPLY: return "Multiply";
        case AInput::DIVIDE: return "Divide";
        case AInput::LEFT: return "Left";
        case AInput::RIGHT: return "Right";
        case AInput::UP: return "Up";
        case AInput::DOWN: return "Down";
        case AInput::NUMPAD0: return "Numpad0";
        case AInput::NUMPAD1: return "Numpad1";
        case AInput::NUMPAD2: return "Numpad2";
        case AInput::NUMPAD3: return "Numpad3";
        case AInput::NUMPAD4: return "Numpad4";
        case AInput::NUMPAD5: return "Numpad5";
        case AInput::NUMPAD6: return "Numpad6";
        case AInput::NUMPAD7: return "Numpad7";
        case AInput::NUMPAD8: return "Numpad8";
        case AInput::NUMPAD9: return "Numpad9";
        case AInput::F1: return "F1";
        case AInput::F2: return "F2";
        case AInput::F3: return "F3";
        case AInput::F4: return "F4";
        case AInput::F5: return "F5";
        case AInput::F6: return "F6";
        case AInput::F7: return "F7";
        case AInput::F8: return "F8";
        case AInput::F9: return "F9";
        case AInput::F10: return "F10";
        case AInput::F11: return "F11";
        case AInput::F12: return "F12";
        case AInput::F13: return "F13";
        case AInput::F14: return "F14";
        case AInput::F15: return "F15";
        case AInput::PAUSE: return "Pause";
        case AInput::LBUTTON: return "LButton";
        case AInput::RBUTTON: return "RButton";
        case AInput::CBUTTON: return "CButton";
    }
}

bool AInput::isKeyDown(Key k) {
    if (gKeyStateForTesting[k]) {
        return true;
    }
    return AInput::native::isKeyDown(k);
}

void AInput::overrideStateForTesting(AInput::Key k, bool value) {
    gKeyStateForTesting[k] = value;
}
