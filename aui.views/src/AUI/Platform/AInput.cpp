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

#include "AInput.h"

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
        case AInput::Num0: return "Num0";
        case AInput::Num1: return "Num1";
        case AInput::Num2: return "Num2";
        case AInput::Num3: return "Num3";
        case AInput::Num4: return "Num4";
        case AInput::Num5: return "Num5";
        case AInput::Num6: return "Num6";
        case AInput::Num7: return "Num7";
        case AInput::Num8: return "Num8";
        case AInput::Num9: return "Num9";
        case AInput::Escape: return "Escape";
        case AInput::LControl: case AInput::RControl:  return "Ctrl";
        case AInput::LShift: case AInput::RShift: return "Shift";
        case AInput::LAlt: case AInput::RAlt: return "Alt";
        case AInput::LSystem: case AInput::RSystem: return "Win";
        case AInput::Menu: return "Menu";
        case AInput::LBracket: return "LBracket";
        case AInput::RBracket: return "RBracket";
        case AInput::SemiColon: return "SemiColon";
        case AInput::Comma: return "Comma";
        case AInput::Period: return "Period";
        case AInput::Quote: return "Quote";
        case AInput::Slash: return "Slash";
        case AInput::BackSlash: return "BackSlash";
        case AInput::Tilde: return "Tilde";
        case AInput::Equal: return "Equal";
        case AInput::Dash: return "Dash";
        case AInput::Space: return "Space";
        case AInput::Return: return "Return";
        case AInput::BackSpace: return "BackSpace";
        case AInput::Tab: return "Tab";
        case AInput::PageUp: return "PageUp";
        case AInput::PageDown: return "PageDown";
        case AInput::End: return "End";
        case AInput::Home: return "Home";
        case AInput::Insert: return "Insert";
        case AInput::Delete: return "Delete";
        case AInput::Add: return "Add";
        case AInput::Subtract: return "Subtract";
        case AInput::Multiply: return "Multiply";
        case AInput::Divide: return "Divide";
        case AInput::Left: return "Left";
        case AInput::Right: return "Right";
        case AInput::Up: return "Up";
        case AInput::Down: return "Down";
        case AInput::Numpad0: return "Numpad0";
        case AInput::Numpad1: return "Numpad1";
        case AInput::Numpad2: return "Numpad2";
        case AInput::Numpad3: return "Numpad3";
        case AInput::Numpad4: return "Numpad4";
        case AInput::Numpad5: return "Numpad5";
        case AInput::Numpad6: return "Numpad6";
        case AInput::Numpad7: return "Numpad7";
        case AInput::Numpad8: return "Numpad8";
        case AInput::Numpad9: return "Numpad9";
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
        case AInput::Pause: return "Pause";
        case AInput::LButton: return "LButton";
        case AInput::RButton: return "RButton";
        case AInput::CButton: return "CButton";
    }
}


#if AUI_PLATFORM_WIN
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

#elif AUI_PLATFORM_ANDROID

AInput::Key AInput::fromNative(int k) {
    return Unknown;
}
int AInput::toNative(Key key) {
    return 0;
}
bool AInput::isKeyDown(Key k) {
    return false;
}

#elif AUI_PLATFORM_APPLE

// TODO apple
AInput::Key AInput::fromNative(int w) {
    return AInput::B;
}

int AInput::toNative(AInput::Key w) {
    return 0;
}

bool AInput::isKeyDown(AInput::Key k) {
    return false;
}

#else
#include <X11/Xlib.h>
#include <X11/keysym.h>
extern Display* gDisplay;
bool isMouseKeyDown(AInput::Key button) {
	// we don't care about these but they are required
	::Window root, child;
	int wx, wy;
	int gx, gy;

	unsigned int buttons = 0;
	XQueryPointer(gDisplay, DefaultRootWindow(gDisplay), &root, &child, &gx, &gy, &wx, &wy, &buttons);

	switch (button)
	{
	case AInput::LButton:     return buttons & Button1Mask;
	case AInput::RButton:    return buttons & Button3Mask;
		//scase Key::MButton:   return buttons & Button2Mask;
	default:              return false;
	}

	return false;
}
AInput::Key AInput::fromNative(int k) {
	Key key;
	KeySym keycode = XKeycodeToKeysym(gDisplay, k, 0);
	switch (keycode) {
	case XK_Shift_L: key = AInput::LShift; break;
	case XK_Shift_R: key = AInput::RShift; break;
	case XK_Control_L: key = AInput::LControl; break;
	case XK_Control_R: key = AInput::RControl; break;
	case XK_Alt_L: key = AInput::LAlt; break;
	case XK_Alt_R: key = AInput::RAlt; break;
	case XK_Super_L: key = AInput::LSystem; break;
	case XK_Super_R: key = AInput::RSystem; break;
	case XK_Menu: key = AInput::Menu; break;
	case XK_Escape: key = AInput::Escape; break;
	case XK_semicolon: key = AInput::SemiColon; break;
	case XK_slash: key = AInput::Slash; break;
	case XK_equal: key = AInput::Equal; break;
	case XK_minus: key = AInput::Dash; break;
	case XK_bracketleft: key = AInput::LBracket; break;
	case XK_bracketright: key = AInput::RBracket; break;
	case XK_comma: key = AInput::Comma; break;
	case XK_period: key = AInput::Period; break;
	case XK_apostrophe: key = AInput::Quote; break;
	case XK_backslash: key = AInput::BackSlash; break;
	case XK_grave: key = AInput::Tilde; break;
	case XK_space: key = AInput::Space; break;
	case XK_Return: key = AInput::Return; break;
	case XK_BackSpace: key = AInput::BackSpace; break;
	case XK_Tab: key = AInput::Tab; break;
	case XK_Prior: key = AInput::PageUp; break;
	case XK_Next: key = AInput::PageDown; break;
	case XK_End: key = AInput::End; break;
	case XK_Home: key = AInput::Home; break;
	case XK_Insert: key = AInput::Insert; break;
	case XK_Delete: key = AInput::Delete; break;
	case XK_KP_Add: key = AInput::Add; break;
	case XK_KP_Subtract: key = AInput::Subtract; break;
	case XK_KP_Multiply: key = AInput::Multiply; break;
	case XK_KP_Divide: key = AInput::Divide; break;
	case XK_Pause: key = AInput::Pause; break;
	case XK_F1: key = AInput::F1; break;
	case XK_F2: key = AInput::F2; break;
	case XK_F3: key = AInput::F3; break;
	case XK_F4: key = AInput::F4; break;
	case XK_F5: key = AInput::F5; break;
	case XK_F6: key = AInput::F6; break;
	case XK_F7: key = AInput::F7; break;
	case XK_F8: key = AInput::F8; break;
	case XK_F9: key = AInput::F9; break;
	case XK_F10: key = AInput::F10; break;
	case XK_F11: key = AInput::F11; break;
	case XK_F12: key = AInput::F12; break;
	case XK_F13: key = AInput::F13; break;
	case XK_F14: key = AInput::F14; break;
	case XK_F15: key = AInput::F15; break;
	case XK_Left: key = AInput::Left; break;
	case XK_Right: key = AInput::Right; break;
	case XK_Up: key = AInput::Up; break;
	case XK_Down: key = AInput::Down; break;
	case XK_KP_Insert: key = AInput::Numpad0; break;
	case XK_KP_End: key = AInput::Numpad1; break;
	case XK_KP_Down: key = AInput::Numpad2; break;
	case XK_KP_Page_Down: key = AInput::Numpad3; break;
	case XK_KP_Left: key = AInput::Numpad4; break;
	case XK_KP_Begin: key = AInput::Numpad5; break;
	case XK_KP_Right: key = AInput::Numpad6; break;
	case XK_KP_Home: key = AInput::Numpad7; break;
	case XK_KP_Up: key = AInput::Numpad8; break;
	case XK_KP_Page_Up: key = AInput::Numpad9; break;
	case XK_a: key = AInput::A; break;
	case XK_b: key = AInput::B; break;
	case XK_c: key = AInput::C; break;
	case XK_d: key = AInput::D; break;
	case XK_e: key = AInput::E; break;
	case XK_f: key = AInput::F; break;
	case XK_g: key = AInput::G; break;
	case XK_h: key = AInput::H; break;
	case XK_i: key = AInput::I; break;
	case XK_j: key = AInput::J; break;
	case XK_k: key = AInput::K; break;
	case XK_l: key = AInput::L; break;
	case XK_m: key = AInput::M; break;
	case XK_n: key = AInput::N; break;
	case XK_o: key = AInput::O; break;
	case XK_p: key = AInput::P; break;
	case XK_q: key = AInput::Q; break;
	case XK_r: key = AInput::R; break;
	case XK_s: key = AInput::S; break;
	case XK_t: key = AInput::T; break;
	case XK_u: key = AInput::U; break;
	case XK_v: key = AInput::V; break;
	case XK_w: key = AInput::W; break;
	case XK_x: key = AInput::X; break;
	case XK_y: key = AInput::Y; break;
	case XK_z: key = AInput::Z; break;
	case XK_0: key = AInput::Num0; break;
	case XK_1: key = AInput::Num1; break;
	case XK_2: key = AInput::Num2; break;
	case XK_3: key = AInput::Num3; break;
	case XK_4: key = AInput::Num4; break;
	case XK_5: key = AInput::Num5; break;
	case XK_6: key = AInput::Num6; break;
	case XK_7: key = AInput::Num7; break;
	case XK_8: key = AInput::Num8; break;
	case XK_9: key = AInput::Num9; break;
	default: key = AInput::Unknown; break;
	}
	return key;
}
int AInput::toNative(Key key) {
	int keysym = 0;
	switch (key)
	{
	case AInput::LShift:     keysym = XK_Shift_L;      break;
	case AInput::RShift:     keysym = XK_Shift_R;      break;
	case AInput::LControl:   keysym = XK_Control_L;    break;
	case AInput::RControl:   keysym = XK_Control_R;    break;
	case AInput::LAlt:       keysym = XK_Alt_L;        break;
	case AInput::RAlt:       keysym = XK_Alt_R;        break;
	case AInput::LSystem:    keysym = XK_Super_L;      break;
	case AInput::RSystem:    keysym = XK_Super_R;      break;
	case AInput::Menu:       keysym = XK_Menu;         break;
	case AInput::Escape:     keysym = XK_Escape;       break;
	case AInput::SemiColon:  keysym = XK_semicolon;    break;
	case AInput::Slash:      keysym = XK_slash;        break;
	case AInput::Equal:      keysym = XK_equal;        break;
	case AInput::Dash:       keysym = XK_minus;        break;
	case AInput::LBracket:   keysym = XK_bracketleft;  break;
	case AInput::RBracket:   keysym = XK_bracketright; break;
	case AInput::Comma:      keysym = XK_comma;        break;
	case AInput::Period:     keysym = XK_period;       break;
	case AInput::Quote:      keysym = XK_apostrophe;   break;
	case AInput::BackSlash:  keysym = XK_backslash;    break;
	case AInput::Tilde:      keysym = XK_grave;        break;
	case AInput::Space:      keysym = XK_space;        break;
	case AInput::Return:     keysym = XK_Return;       break;
	case AInput::BackSpace:  keysym = XK_BackSpace;    break;
	case AInput::Tab:        keysym = XK_Tab;          break;
	case AInput::PageUp:     keysym = XK_Prior;        break;
	case AInput::PageDown:   keysym = XK_Next;         break;
	case AInput::End:        keysym = XK_End;          break;
	case AInput::Home:       keysym = XK_Home;         break;
	case AInput::Insert:     keysym = XK_Insert;       break;
	case AInput::Delete:     keysym = XK_Delete;       break;
	case AInput::Add:        keysym = XK_KP_Add;       break;
	case AInput::Subtract:   keysym = XK_KP_Subtract;  break;
	case AInput::Multiply:   keysym = XK_KP_Multiply;  break;
	case AInput::Divide:     keysym = XK_KP_Divide;    break;
	case AInput::Pause:      keysym = XK_Pause;        break;
	case AInput::F1:         keysym = XK_F1;           break;
	case AInput::F2:         keysym = XK_F2;           break;
	case AInput::F3:         keysym = XK_F3;           break;
	case AInput::F4:         keysym = XK_F4;           break;
	case AInput::F5:         keysym = XK_F5;           break;
	case AInput::F6:         keysym = XK_F6;           break;
	case AInput::F7:         keysym = XK_F7;           break;
	case AInput::F8:         keysym = XK_F8;           break;
	case AInput::F9:         keysym = XK_F9;           break;
	case AInput::F10:        keysym = XK_F10;          break;
	case AInput::F11:        keysym = XK_F11;          break;
	case AInput::F12:        keysym = XK_F12;          break;
	case AInput::F13:        keysym = XK_F13;          break;
	case AInput::F14:        keysym = XK_F14;          break;
	case AInput::F15:        keysym = XK_F15;          break;
	case AInput::Left:       keysym = XK_Left;         break;
	case AInput::Right:      keysym = XK_Right;        break;
	case AInput::Up:         keysym = XK_Up;           break;
	case AInput::Down:       keysym = XK_Down;         break;
	case AInput::Numpad0:    keysym = XK_KP_Insert;    break;
	case AInput::Numpad1:    keysym = XK_KP_End;       break;
	case AInput::Numpad2:    keysym = XK_KP_Down;      break;
	case AInput::Numpad3:    keysym = XK_KP_Page_Down; break;
	case AInput::Numpad4:    keysym = XK_KP_Left;      break;
	case AInput::Numpad5:    keysym = XK_KP_Begin;     break;
	case AInput::Numpad6:    keysym = XK_KP_Right;     break;
	case AInput::Numpad7:    keysym = XK_KP_Home;      break;
	case AInput::Numpad8:    keysym = XK_KP_Up;        break;
	case AInput::Numpad9:    keysym = XK_KP_Page_Up;   break;
	case AInput::A:          keysym = XK_a;            break;
	case AInput::B:          keysym = XK_b;            break;
	case AInput::C:          keysym = XK_c;            break;
	case AInput::D:          keysym = XK_d;            break;
	case AInput::E:          keysym = XK_e;            break;
	case AInput::F:          keysym = XK_f;            break;
	case AInput::G:          keysym = XK_g;            break;
	case AInput::H:          keysym = XK_h;            break;
	case AInput::I:          keysym = XK_i;            break;
	case AInput::J:          keysym = XK_j;            break;
	case AInput::K:          keysym = XK_k;            break;
	case AInput::L:          keysym = XK_l;            break;
	case AInput::M:          keysym = XK_m;            break;
	case AInput::N:          keysym = XK_n;            break;
	case AInput::O:          keysym = XK_o;            break;
	case AInput::P:          keysym = XK_p;            break;
	case AInput::Q:          keysym = XK_q;            break;
	case AInput::R:          keysym = XK_r;            break;
	case AInput::S:          keysym = XK_s;            break;
	case AInput::T:          keysym = XK_t;            break;
	case AInput::U:          keysym = XK_u;            break;
	case AInput::V:          keysym = XK_v;            break;
	case AInput::W:          keysym = XK_w;            break;
	case AInput::X:          keysym = XK_x;            break;
	case AInput::Y:          keysym = XK_y;            break;
	case AInput::Z:          keysym = XK_z;            break;
	case AInput::Num0:       keysym = XK_0;            break;
	case AInput::Num1:       keysym = XK_1;            break;
	case AInput::Num2:       keysym = XK_2;            break;
	case AInput::Num3:       keysym = XK_3;            break;
	case AInput::Num4:       keysym = XK_4;            break;
	case AInput::Num5:       keysym = XK_5;            break;
	case AInput::Num6:       keysym = XK_6;            break;
	case AInput::Num7:       keysym = XK_7;            break;
	case AInput::Num8:       keysym = XK_8;            break;
	case AInput::Num9:       keysym = XK_9;            break;
	default:                   keysym = 0;               break;
	}
	return keysym;
}
bool AInput::isKeyDown(Key k) {
	if (k == Key::LButton || k == Key::RButton) {
		return isMouseKeyDown(k);
	}
	auto keysym = (KeySym)toNative(k);

	// Convert to keycode
	KeyCode keycode = XKeysymToKeycode(gDisplay, keysym);
	if (keycode != 0)
	{
		// Get the whole keyboard state
		char keys[32];
		XQueryKeymap(gDisplay, keys);

		// Check our keycode
		return (keys[keycode / 8] & (1 << (keycode % 8))) != 0;
	}
	else
	{
		return false;
	}
}
#endif