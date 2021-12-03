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