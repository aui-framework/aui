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

#include <windows.h>

AInput::Key AInput::fromNative(int key) {
	Key vkey;
	switch (key)
	{
	default:           vkey = AInput::UNKNOWN;     break;
	case 'A':          vkey = AInput::A;           break;
	case 'B':          vkey = AInput::B;           break;
	case 'C':          vkey = AInput::C;           break;
	case 'D':          vkey = AInput::D;           break;
	case 'E':          vkey = AInput::E;           break;
	case 'F':          vkey = AInput::F;           break;
	case 'G':          vkey = AInput::G;           break;
	case 'H':          vkey = AInput::H;           break;
	case 'I':          vkey = AInput::I;           break;
	case 'J':          vkey = AInput::J;           break;
	case 'K':          vkey = AInput::K;           break;
	case 'L':          vkey = AInput::L;           break;
	case 'M':          vkey = AInput::M;           break;
	case 'N':          vkey = AInput::N;           break;
	case 'O':          vkey = AInput::O;           break;
	case 'P':          vkey = AInput::P;           break;
	case 'Q':          vkey = AInput::Q;           break;
	case 'R':          vkey = AInput::R;           break;
	case 'S':          vkey = AInput::S;           break;
	case 'T':          vkey = AInput::T;           break;
	case 'U':          vkey = AInput::U;           break;
	case 'V':          vkey = AInput::V;           break;
	case 'W':          vkey = AInput::W;           break;
	case 'X':          vkey = AInput::X;           break;
	case 'Y':          vkey = AInput::Y;           break;
	case 'Z':          vkey = AInput::Z;           break;
	case '0':       vkey = AInput::NUM0;           break;
	case '1':       vkey = AInput::NUM1;           break;
	case '2':       vkey = AInput::NUM2;           break;
	case '3':       vkey = AInput::NUM3;           break;
	case '4':       vkey = AInput::NUM4;           break;
	case '5':       vkey = AInput::NUM5;           break;
	case '6':       vkey = AInput::NUM6;           break;
	case '7':       vkey = AInput::NUM7;           break;
	case '8':       vkey = AInput::NUM8;           break;
	case '9':       vkey = AInput::NUM9;           break;
	case VK_ESCAPE:     vkey = AInput::ESCAPE;     break;
	case VK_CONTROL:
	case VK_LCONTROL:   vkey = AInput::LCONTROL;   break;
	case VK_SHIFT:     vkey = AInput::LSHIFT;     break;
	case VK_LSHIFT:     vkey = AInput::LSHIFT;     break;
	case VK_MENU:       vkey = AInput::LALT;      break;
	case VK_LMENU:       vkey = AInput::LALT;      break;
	case VK_LWIN:    vkey = AInput::LSYSTEM;       break;
	case VK_RCONTROL:   vkey = AInput::RCONTROL;   break;
	case VK_RSHIFT:     vkey = AInput::RSHIFT;     break;
	case VK_RMENU:       vkey = AInput::RALT;      break;
	case VK_RWIN:    vkey = AInput::RSYSTEM;       break;
	case VK_APPS:       vkey = AInput::MENU;       break;
	case VK_OEM_4:   vkey = AInput::LBRACKET;      break;
	case VK_OEM_6:   vkey = AInput::RBRACKET;      break;
	case VK_OEM_1:  vkey = AInput::SEMICOLON;      break;
	case VK_OEM_COMMA:      vkey = AInput::COMMA;  break;
	case VK_OEM_PERIOD:     vkey = AInput::PERIOD; break;
	case VK_OEM_7:      vkey = AInput::QUOTE;      break;
	case VK_OEM_2:      vkey = AInput::SLASH;      break;
	case VK_OEM_5:  vkey = AInput::BACKSLASH;      break;
	case VK_OEM_3:      vkey = AInput::TILDE;      break;
	case VK_OEM_PLUS:      vkey = AInput::EQUAL;   break;
	case VK_OEM_MINUS:       vkey = AInput::DASH;  break;
	case VK_SPACE:      vkey = AInput::SPACE;      break;
	case VK_RETURN:     vkey = AInput::RETURN;     break;
	case VK_BACK:  vkey = AInput::BACKSPACE;       break;
	case VK_TAB:        vkey = AInput::TAB;        break;
	case VK_PRIOR:     vkey = AInput::PAGEUP;      break;
	case VK_NEXT:   vkey = AInput::PAGEDOWN;       break;
	case VK_END:        vkey = AInput::END;        break;
	case VK_HOME:       vkey = AInput::HOME;       break;
	case VK_INSERT:     vkey = AInput::INSERT;     break;
	case VK_DELETE:     vkey = AInput::DEL;        break;
	case VK_ADD:        vkey = AInput::ADD;        break;
	case VK_SUBTRACT:   vkey = AInput::SUBTRACT;   break;
	case VK_MULTIPLY:   vkey = AInput::MULTIPLY;   break;
	case VK_DIVIDE:     vkey = AInput::DIVIDE;     break;
	case VK_LEFT:       vkey = AInput::LEFT;       break;
	case VK_RIGHT:      vkey = AInput::RIGHT;      break;
	case VK_UP:         vkey = AInput::UP;         break;
	case VK_DOWN:       vkey = AInput::DOWN;       break;
	case VK_NUMPAD0:    vkey = AInput::NUMPAD0;    break;
	case VK_NUMPAD1:    vkey = AInput::NUMPAD1;    break;
	case VK_NUMPAD2:    vkey = AInput::NUMPAD2;    break;
	case VK_NUMPAD3:    vkey = AInput::NUMPAD3;    break;
	case VK_NUMPAD4:    vkey = AInput::NUMPAD4;    break;
	case VK_NUMPAD5:    vkey = AInput::NUMPAD5;    break;
	case VK_NUMPAD6:    vkey = AInput::NUMPAD6;    break;
	case VK_NUMPAD7:    vkey = AInput::NUMPAD7;    break;
	case VK_NUMPAD8:    vkey = AInput::NUMPAD8;    break;
	case VK_NUMPAD9:    vkey = AInput::NUMPAD9;    break;
	case VK_F1:         vkey = AInput::F1;         break;
	case VK_F2:         vkey = AInput::F2;         break;
	case VK_F3:         vkey = AInput::F3;         break;
	case VK_F4:         vkey = AInput::F4;         break;
	case VK_F5:         vkey = AInput::F5;         break;
	case VK_F6:         vkey = AInput::F6;         break;
	case VK_F7:         vkey = AInput::F7;         break;
	case VK_F8:         vkey = AInput::F8;         break;
	case VK_F9:         vkey = AInput::F9;         break;
	case VK_F10:        vkey = AInput::F10;        break;
	case VK_F11:        vkey = AInput::F11;        break;
	case VK_F12:        vkey = AInput::F12;        break;
	case VK_F13:        vkey = AInput::F13;        break;
	case VK_F14:        vkey = AInput::F14;        break;
	case VK_F15:        vkey = AInput::F15;        break;
	case VK_SNAPSHOT:   vkey = AInput::PRINTSCREEN;break;
	case VK_PAUSE:      vkey = AInput::PAUSE;      break;
	case VK_LBUTTON:    vkey = AInput::LBUTTON;    break;
	case VK_RBUTTON:    vkey = AInput::RBUTTON;    break;
	case VK_MBUTTON:    vkey = AInput::CBUTTON;    break;
	}
	return vkey;
}
int AInput::toNative(Key key) {
	int vkey = 0;
	switch (key)
	{
	default:                   vkey = 0;             break;
	case AInput::A: vkey = 'A';           break;
	case AInput::B: vkey = 'B';           break;
	case AInput::C: vkey = 'C';           break;
	case AInput::D: vkey = 'D';           break;
	case AInput::E: vkey = 'E';           break;
	case AInput::F: vkey = 'F';           break;
	case AInput::G: vkey = 'G';           break;
	case AInput::H: vkey = 'H';           break;
	case AInput::I: vkey = 'I';           break;
	case AInput::J: vkey = 'J';           break;
	case AInput::K: vkey = 'K';           break;
	case AInput::L: vkey = 'L';           break;
	case AInput::M: vkey = 'M';           break;
	case AInput::N: vkey = 'N';           break;
	case AInput::O: vkey = 'O';           break;
	case AInput::P: vkey = 'P';           break;
	case AInput::Q: vkey = 'Q';           break;
	case AInput::R: vkey = 'R';           break;
	case AInput::S: vkey = 'S';           break;
	case AInput::T: vkey = 'T';           break;
	case AInput::U: vkey = 'U';           break;
	case AInput::V: vkey = 'V';           break;
	case AInput::W: vkey = 'W';           break;
	case AInput::X: vkey = 'X';           break;
	case AInput::Y: vkey = 'Y';           break;
	case AInput::Z: vkey = 'Z';           break;
	case AInput::NUM0: vkey = '0';           break;
	case AInput::NUM1: vkey = '1';           break;
	case AInput::NUM2: vkey = '2';           break;
	case AInput::NUM3: vkey = '3';           break;
	case AInput::NUM4: vkey = '4';           break;
	case AInput::NUM5: vkey = '5';           break;
	case AInput::NUM6: vkey = '6';           break;
	case AInput::NUM7: vkey = '7';           break;
	case AInput::NUM8: vkey = '8';           break;
	case AInput::NUM9: vkey = '9';           break;
	case AInput::ESCAPE: vkey = VK_ESCAPE;     break;
	case AInput::LCONTROL: vkey = VK_LCONTROL;   break;
	case AInput::LSHIFT: vkey = VK_LSHIFT;     break;
	case AInput::LALT: vkey = VK_LMENU;      break;
	case AInput::LSYSTEM: vkey = VK_LWIN;       break;
	case AInput::RCONTROL: vkey = VK_RCONTROL;   break;
	case AInput::RSHIFT: vkey = VK_RSHIFT;     break;
	case AInput::RALT: vkey = VK_RMENU;      break;
	case AInput::RSYSTEM: vkey = VK_RWIN;       break;
	case AInput::MENU: vkey = VK_APPS;       break;
	case AInput::LBRACKET: vkey = VK_OEM_4;      break;
	case AInput::RBRACKET: vkey = VK_OEM_6;      break;
	case AInput::SEMICOLON: vkey = VK_OEM_1;      break;
	case AInput::COMMA: vkey = VK_OEM_COMMA;  break;
	case AInput::PERIOD: vkey = VK_OEM_PERIOD; break;
	case AInput::QUOTE: vkey = VK_OEM_7;      break;
	case AInput::SLASH: vkey = VK_OEM_2;      break;
	case AInput::BACKSLASH: vkey = VK_OEM_5;      break;
	case AInput::TILDE: vkey = VK_OEM_3;      break;
	case AInput::EQUAL: vkey = VK_OEM_PLUS;   break;
	case AInput::DASH: vkey = VK_OEM_MINUS;  break;
	case AInput::SPACE: vkey = VK_SPACE;      break;
	case AInput::RETURN: vkey = VK_RETURN;     break;
	case AInput::BACKSPACE: vkey = VK_BACK;       break;
	case AInput::TAB: vkey = VK_TAB;        break;
	case AInput::PAGEUP: vkey = VK_PRIOR;      break;
	case AInput::PAGEDOWN: vkey = VK_NEXT;       break;
	case AInput::END: vkey = VK_END;        break;
	case AInput::HOME: vkey = VK_HOME;       break;
	case AInput::INSERT: vkey = VK_INSERT;     break;
	case AInput::DEL: vkey = VK_DELETE;     break;
	case AInput::ADD: vkey = VK_ADD;        break;
	case AInput::SUBTRACT: vkey = VK_SUBTRACT;   break;
	case AInput::MULTIPLY: vkey = VK_MULTIPLY;   break;
	case AInput::DIVIDE: vkey = VK_DIVIDE;     break;
	case AInput::LEFT: vkey = VK_LEFT;       break;
	case AInput::RIGHT: vkey = VK_RIGHT;      break;
	case AInput::UP: vkey = VK_UP;         break;
	case AInput::DOWN: vkey = VK_DOWN;       break;
	case AInput::NUMPAD0: vkey = VK_NUMPAD0;    break;
	case AInput::NUMPAD1: vkey = VK_NUMPAD1;    break;
	case AInput::NUMPAD2: vkey = VK_NUMPAD2;    break;
	case AInput::NUMPAD3: vkey = VK_NUMPAD3;    break;
	case AInput::NUMPAD4: vkey = VK_NUMPAD4;    break;
	case AInput::NUMPAD5: vkey = VK_NUMPAD5;    break;
	case AInput::NUMPAD6: vkey = VK_NUMPAD6;    break;
	case AInput::NUMPAD7: vkey = VK_NUMPAD7;    break;
	case AInput::NUMPAD8: vkey = VK_NUMPAD8;    break;
	case AInput::NUMPAD9: vkey = VK_NUMPAD9;    break;
	case AInput::F1: vkey = VK_F1;         break;
	case AInput::F2: vkey = VK_F2;         break;
	case AInput::F3: vkey = VK_F3;         break;
	case AInput::F4: vkey = VK_F4;         break;
	case AInput::F5: vkey = VK_F5;         break;
	case AInput::F6: vkey = VK_F6;         break;
	case AInput::F7: vkey = VK_F7;         break;
	case AInput::F8: vkey = VK_F8;         break;
	case AInput::F9: vkey = VK_F9;         break;
	case AInput::F10: vkey = VK_F10;        break;
	case AInput::F11: vkey = VK_F11;        break;
	case AInput::F12: vkey = VK_F12;        break;
	case AInput::F13: vkey = VK_F13;        break;
	case AInput::F14: vkey = VK_F14;        break;
	case AInput::F15: vkey = VK_F15;        break;
	case AInput::PAUSE: vkey = VK_PAUSE;      break;
	case AInput::LBUTTON: vkey = VK_LBUTTON;    break;
	case AInput::RBUTTON: vkey = VK_RBUTTON;    break;
	case AInput::CBUTTON: vkey = VK_MBUTTON;    break;
	}
	return vkey;
}

bool AInput::native::isKeyDown(Key k) {
	return GetAsyncKeyState(toNative(k)) & 32768;
}
