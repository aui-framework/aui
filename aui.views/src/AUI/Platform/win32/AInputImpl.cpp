/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include <AUI/Platform/AInput.h>

#include <windows.h>

AInput::Key AInput::fromNative(int key) {
	Key vkey;
	switch (key)
	{
	default:           vkey = AInput::Unknown;     break;
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
	case '0':       vkey = AInput::Num0;           break;
	case '1':       vkey = AInput::Num1;           break;
	case '2':       vkey = AInput::Num2;           break;
	case '3':       vkey = AInput::Num3;           break;
	case '4':       vkey = AInput::Num4;           break;
	case '5':       vkey = AInput::Num5;           break;
	case '6':       vkey = AInput::Num6;           break;
	case '7':       vkey = AInput::Num7;           break;
	case '8':       vkey = AInput::Num8;           break;
	case '9':       vkey = AInput::Num9;           break;
	case VK_ESCAPE:     vkey = AInput::Escape;     break;
	case VK_CONTROL:
	case VK_LCONTROL:   vkey = AInput::LControl;   break;
	case VK_SHIFT:     vkey = AInput::LShift;     break;
	case VK_LSHIFT:     vkey = AInput::LShift;     break;
	case VK_MENU:       vkey = AInput::LAlt;      break;
	case VK_LMENU:       vkey = AInput::LAlt;      break;
	case VK_LWIN:    vkey = AInput::LSystem;       break;
	case VK_RCONTROL:   vkey = AInput::RControl;   break;
	case VK_RSHIFT:     vkey = AInput::RShift;     break;
	case VK_RMENU:       vkey = AInput::RAlt;      break;
	case VK_RWIN:    vkey = AInput::RSystem;       break;
	case VK_APPS:       vkey = AInput::Menu;       break;
	case VK_OEM_4:   vkey = AInput::LBracket;      break;
	case VK_OEM_6:   vkey = AInput::RBracket;      break;
	case VK_OEM_1:  vkey = AInput::SemiColon;      break;
	case VK_OEM_COMMA:      vkey = AInput::Comma;  break;
	case VK_OEM_PERIOD:     vkey = AInput::Period; break;
	case VK_OEM_7:      vkey = AInput::Quote;      break;
	case VK_OEM_2:      vkey = AInput::Slash;      break;
	case VK_OEM_5:  vkey = AInput::BackSlash;      break;
	case VK_OEM_3:      vkey = AInput::Tilde;      break;
	case VK_OEM_PLUS:      vkey = AInput::Equal;   break;
	case VK_OEM_MINUS:       vkey = AInput::Dash;  break;
	case VK_SPACE:      vkey = AInput::Space;      break;
	case VK_RETURN:     vkey = AInput::Return;     break;
	case VK_BACK:  vkey = AInput::BackSpace;       break;
	case VK_TAB:        vkey = AInput::Tab;        break;
	case VK_PRIOR:     vkey = AInput::PageUp;      break;
	case VK_NEXT:   vkey = AInput::PageDown;       break;
	case VK_END:        vkey = AInput::End;        break;
	case VK_HOME:       vkey = AInput::Home;       break;
	case VK_INSERT:     vkey = AInput::Insert;     break;
	case VK_DELETE:     vkey = AInput::Delete;     break;
	case VK_ADD:        vkey = AInput::Add;        break;
	case VK_SUBTRACT:   vkey = AInput::Subtract;   break;
	case VK_MULTIPLY:   vkey = AInput::Multiply;   break;
	case VK_DIVIDE:     vkey = AInput::Divide;     break;
	case VK_LEFT:       vkey = AInput::Left;       break;
	case VK_RIGHT:      vkey = AInput::Right;      break;
	case VK_UP:         vkey = AInput::Up;         break;
	case VK_DOWN:       vkey = AInput::Down;       break;
	case VK_NUMPAD0:    vkey = AInput::Numpad0;    break;
	case VK_NUMPAD1:    vkey = AInput::Numpad1;    break;
	case VK_NUMPAD2:    vkey = AInput::Numpad2;    break;
	case VK_NUMPAD3:    vkey = AInput::Numpad3;    break;
	case VK_NUMPAD4:    vkey = AInput::Numpad4;    break;
	case VK_NUMPAD5:    vkey = AInput::Numpad5;    break;
	case VK_NUMPAD6:    vkey = AInput::Numpad6;    break;
	case VK_NUMPAD7:    vkey = AInput::Numpad7;    break;
	case VK_NUMPAD8:    vkey = AInput::Numpad8;    break;
	case VK_NUMPAD9:    vkey = AInput::Numpad9;    break;
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
	case VK_PAUSE:      vkey = AInput::Pause;      break;
	case VK_LBUTTON:    vkey = AInput::LButton;    break;
	case VK_RBUTTON:    vkey = AInput::RButton;    break;
	case VK_MBUTTON:    vkey = AInput::CButton;    break;
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
	case AInput::Num0: vkey = '0';           break;
	case AInput::Num1: vkey = '1';           break;
	case AInput::Num2: vkey = '2';           break;
	case AInput::Num3: vkey = '3';           break;
	case AInput::Num4: vkey = '4';           break;
	case AInput::Num5: vkey = '5';           break;
	case AInput::Num6: vkey = '6';           break;
	case AInput::Num7: vkey = '7';           break;
	case AInput::Num8: vkey = '8';           break;
	case AInput::Num9: vkey = '9';           break;
	case AInput::Escape: vkey = VK_ESCAPE;     break;
	case AInput::LControl: vkey = VK_LCONTROL;   break;
	case AInput::LShift: vkey = VK_LSHIFT;     break;
	case AInput::LAlt: vkey = VK_LMENU;      break;
	case AInput::LSystem: vkey = VK_LWIN;       break;
	case AInput::RControl: vkey = VK_RCONTROL;   break;
	case AInput::RShift: vkey = VK_RSHIFT;     break;
	case AInput::RAlt: vkey = VK_RMENU;      break;
	case AInput::RSystem: vkey = VK_RWIN;       break;
	case AInput::Menu: vkey = VK_APPS;       break;
	case AInput::LBracket: vkey = VK_OEM_4;      break;
	case AInput::RBracket: vkey = VK_OEM_6;      break;
	case AInput::SemiColon: vkey = VK_OEM_1;      break;
	case AInput::Comma: vkey = VK_OEM_COMMA;  break;
	case AInput::Period: vkey = VK_OEM_PERIOD; break;
	case AInput::Quote: vkey = VK_OEM_7;      break;
	case AInput::Slash: vkey = VK_OEM_2;      break;
	case AInput::BackSlash: vkey = VK_OEM_5;      break;
	case AInput::Tilde: vkey = VK_OEM_3;      break;
	case AInput::Equal: vkey = VK_OEM_PLUS;   break;
	case AInput::Dash: vkey = VK_OEM_MINUS;  break;
	case AInput::Space: vkey = VK_SPACE;      break;
	case AInput::Return: vkey = VK_RETURN;     break;
	case AInput::BackSpace: vkey = VK_BACK;       break;
	case AInput::Tab: vkey = VK_TAB;        break;
	case AInput::PageUp: vkey = VK_PRIOR;      break;
	case AInput::PageDown: vkey = VK_NEXT;       break;
	case AInput::End: vkey = VK_END;        break;
	case AInput::Home: vkey = VK_HOME;       break;
	case AInput::Insert: vkey = VK_INSERT;     break;
	case AInput::Delete: vkey = VK_DELETE;     break;
	case AInput::Add: vkey = VK_ADD;        break;
	case AInput::Subtract: vkey = VK_SUBTRACT;   break;
	case AInput::Multiply: vkey = VK_MULTIPLY;   break;
	case AInput::Divide: vkey = VK_DIVIDE;     break;
	case AInput::Left: vkey = VK_LEFT;       break;
	case AInput::Right: vkey = VK_RIGHT;      break;
	case AInput::Up: vkey = VK_UP;         break;
	case AInput::Down: vkey = VK_DOWN;       break;
	case AInput::Numpad0: vkey = VK_NUMPAD0;    break;
	case AInput::Numpad1: vkey = VK_NUMPAD1;    break;
	case AInput::Numpad2: vkey = VK_NUMPAD2;    break;
	case AInput::Numpad3: vkey = VK_NUMPAD3;    break;
	case AInput::Numpad4: vkey = VK_NUMPAD4;    break;
	case AInput::Numpad5: vkey = VK_NUMPAD5;    break;
	case AInput::Numpad6: vkey = VK_NUMPAD6;    break;
	case AInput::Numpad7: vkey = VK_NUMPAD7;    break;
	case AInput::Numpad8: vkey = VK_NUMPAD8;    break;
	case AInput::Numpad9: vkey = VK_NUMPAD9;    break;
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
	case AInput::Pause: vkey = VK_PAUSE;      break;
	case AInput::LButton: vkey = VK_LBUTTON;    break;
	case AInput::RButton: vkey = VK_RBUTTON;    break;
	case AInput::CButton: vkey = VK_MBUTTON;    break;
	}
	return vkey;
}

bool AInput::isKeyDown(Key k) {
	return GetAsyncKeyState(toNative(k)) & 32768;
}
