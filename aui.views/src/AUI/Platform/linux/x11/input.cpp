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

#include <AUI/Platform/linux/x11/PlatformAbstractionX11.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

static bool isMouseKeyDown(AInput::Key button) {
    if (PlatformAbstractionX11::ourDisplay == nullptr)
        return false;
    // we don't care about these but they are required
    ::Window root, child;
    int wx, wy;
    int gx, gy;

    unsigned int buttons = 0;
    XQueryPointer(
        PlatformAbstractionX11::ourDisplay, DefaultRootWindow(PlatformAbstractionX11::ourDisplay), &root, &child, &gx,
        &gy, &wx, &wy, &buttons);

    switch (button) {
        case AInput::LBUTTON:
            return buttons & Button1Mask;
        case AInput::RBUTTON:
            return buttons & Button3Mask;
            // scase Key::MButton:   return buttons & Button2Mask;
        default:
            return false;
    }

    return false;
}

AInput::Key PlatformAbstractionX11::inputFromNative(int k) {
    if (PlatformAbstractionX11::ourDisplay == nullptr)
        return AInput::UNKNOWN;
    AInput::Key key;
    KeySym keycode = XkbKeycodeToKeysym(PlatformAbstractionX11::ourDisplay, k, 0, 0); // NOLINT
    switch (keycode) {
        case XK_Shift_L:
            key = AInput::LSHIFT;
            break;
        case XK_Shift_R:
            key = AInput::RSHIFT;
            break;
        case XK_Control_L:
            key = AInput::LCONTROL;
            break;
        case XK_Control_R:
            key = AInput::RCONTROL;
            break;
        case XK_Alt_L:
            key = AInput::LALT;
            break;
        case XK_Alt_R:
            key = AInput::RALT;
            break;
        case XK_Super_L:
            key = AInput::LSYSTEM;
            break;
        case XK_Super_R:
            key = AInput::RSYSTEM;
            break;
        case XK_Menu:
            key = AInput::MENU;
            break;
        case XK_Escape:
            key = AInput::ESCAPE;
            break;
        case XK_semicolon:
            key = AInput::SEMICOLON;
            break;
        case XK_slash:
            key = AInput::SLASH;
            break;
        case XK_equal:
            key = AInput::EQUAL;
            break;
        case XK_minus:
            key = AInput::DASH;
            break;
        case XK_bracketleft:
            key = AInput::LBRACKET;
            break;
        case XK_bracketright:
            key = AInput::RBRACKET;
            break;
        case XK_comma:
            key = AInput::COMMA;
            break;
        case XK_period:
            key = AInput::PERIOD;
            break;
        case XK_apostrophe:
            key = AInput::QUOTE;
            break;
        case XK_backslash:
            key = AInput::BACKSLASH;
            break;
        case XK_grave:
            key = AInput::TILDE;
            break;
        case XK_space:
            key = AInput::SPACE;
            break;
        case XK_Return:
            key = AInput::RETURN;
            break;
        case XK_BackSpace:
            key = AInput::BACKSPACE;
            break;
        case XK_Tab:
            key = AInput::TAB;
            break;
        case XK_Prior:
            key = AInput::PAGEUP;
            break;
        case XK_Next:
            key = AInput::PAGEDOWN;
            break;
        case XK_End:
            key = AInput::END;
            break;
        case XK_Home:
            key = AInput::HOME;
            break;
        case XK_Insert:
            key = AInput::INSERT;
            break;
        case XK_Delete:
            key = AInput::DEL;
            break;
        case XK_KP_Add:
            key = AInput::ADD;
            break;
        case XK_KP_Subtract:
            key = AInput::SUBTRACT;
            break;
        case XK_KP_Multiply:
            key = AInput::MULTIPLY;
            break;
        case XK_KP_Divide:
            key = AInput::DIVIDE;
            break;
        case XK_Pause:
            key = AInput::PAUSE;
            break;
        case XK_F1:
            key = AInput::F1;
            break;
        case XK_F2:
            key = AInput::F2;
            break;
        case XK_F3:
            key = AInput::F3;
            break;
        case XK_F4:
            key = AInput::F4;
            break;
        case XK_F5:
            key = AInput::F5;
            break;
        case XK_F6:
            key = AInput::F6;
            break;
        case XK_F7:
            key = AInput::F7;
            break;
        case XK_F8:
            key = AInput::F8;
            break;
        case XK_F9:
            key = AInput::F9;
            break;
        case XK_F10:
            key = AInput::F10;
            break;
        case XK_F11:
            key = AInput::F11;
            break;
        case XK_F12:
            key = AInput::F12;
            break;
        case XK_F13:
            key = AInput::F13;
            break;
        case XK_F14:
            key = AInput::F14;
            break;
        case XK_F15:
            key = AInput::F15;
            break;
        case XK_Left:
            key = AInput::LEFT;
            break;
        case XK_Right:
            key = AInput::RIGHT;
            break;
        case XK_Up:
            key = AInput::UP;
            break;
        case XK_Down:
            key = AInput::DOWN;
            break;
        case XK_KP_Insert:
            key = AInput::NUMPAD_0;
            break;
        case XK_KP_End:
            key = AInput::NUMPAD_1;
            break;
        case XK_KP_Down:
            key = AInput::NUMPAD_2;
            break;
        case XK_KP_Page_Down:
            key = AInput::NUMPAD_3;
            break;
        case XK_KP_Left:
            key = AInput::NUMPAD_4;
            break;
        case XK_KP_Begin:
            key = AInput::NUMPAD_5;
            break;
        case XK_KP_Right:
            key = AInput::NUMPAD_6;
            break;
        case XK_KP_Home:
            key = AInput::NUMPAD_7;
            break;
        case XK_KP_Up:
            key = AInput::NUMPAD_8;
            break;
        case XK_KP_Page_Up:
            key = AInput::NUMPAD_9;
            break;
        case XK_a:
            key = AInput::A;
            break;
        case XK_b:
            key = AInput::B;
            break;
        case XK_c:
            key = AInput::C;
            break;
        case XK_d:
            key = AInput::D;
            break;
        case XK_e:
            key = AInput::E;
            break;
        case XK_f:
            key = AInput::F;
            break;
        case XK_g:
            key = AInput::G;
            break;
        case XK_h:
            key = AInput::H;
            break;
        case XK_i:
            key = AInput::I;
            break;
        case XK_j:
            key = AInput::J;
            break;
        case XK_k:
            key = AInput::K;
            break;
        case XK_l:
            key = AInput::L;
            break;
        case XK_m:
            key = AInput::M;
            break;
        case XK_n:
            key = AInput::N;
            break;
        case XK_o:
            key = AInput::O;
            break;
        case XK_p:
            key = AInput::P;
            break;
        case XK_q:
            key = AInput::Q;
            break;
        case XK_r:
            key = AInput::R;
            break;
        case XK_s:
            key = AInput::S;
            break;
        case XK_t:
            key = AInput::T;
            break;
        case XK_u:
            key = AInput::U;
            break;
        case XK_v:
            key = AInput::V;
            break;
        case XK_w:
            key = AInput::W;
            break;
        case XK_x:
            key = AInput::X;
            break;
        case XK_y:
            key = AInput::Y;
            break;
        case XK_z:
            key = AInput::Z;
            break;
        case XK_0:
            key = AInput::NUM0;
            break;
        case XK_1:
            key = AInput::NUM1;
            break;
        case XK_2:
            key = AInput::NUM2;
            break;
        case XK_3:
            key = AInput::NUM3;
            break;
        case XK_4:
            key = AInput::NUM4;
            break;
        case XK_5:
            key = AInput::NUM5;
            break;
        case XK_6:
            key = AInput::NUM6;
            break;
        case XK_7:
            key = AInput::NUM7;
            break;
        case XK_8:
            key = AInput::NUM8;
            break;
        case XK_9:
            key = AInput::NUM9;
            break;
        default:
            key = AInput::UNKNOWN;
            break;
    }
    return key;
}

int PlatformAbstractionX11::inputToNative(AInput::Key key) {
    int keysym = 0;
    switch (key) {
        case AInput::LSHIFT:
            keysym = XK_Shift_L;
            break;
        case AInput::RSHIFT:
            keysym = XK_Shift_R;
            break;
        case AInput::LCONTROL:
            keysym = XK_Control_L;
            break;
        case AInput::RCONTROL:
            keysym = XK_Control_R;
            break;
        case AInput::LALT:
            keysym = XK_Alt_L;
            break;
        case AInput::RALT:
            keysym = XK_Alt_R;
            break;
        case AInput::LSYSTEM:
            keysym = XK_Super_L;
            break;
        case AInput::RSYSTEM:
            keysym = XK_Super_R;
            break;
        case AInput::MENU:
            keysym = XK_Menu;
            break;
        case AInput::ESCAPE:
            keysym = XK_Escape;
            break;
        case AInput::SEMICOLON:
            keysym = XK_semicolon;
            break;
        case AInput::SLASH:
            keysym = XK_slash;
            break;
        case AInput::EQUAL:
            keysym = XK_equal;
            break;
        case AInput::DASH:
            keysym = XK_minus;
            break;
        case AInput::LBRACKET:
            keysym = XK_bracketleft;
            break;
        case AInput::RBRACKET:
            keysym = XK_bracketright;
            break;
        case AInput::COMMA:
            keysym = XK_comma;
            break;
        case AInput::PERIOD:
            keysym = XK_period;
            break;
        case AInput::QUOTE:
            keysym = XK_apostrophe;
            break;
        case AInput::BACKSLASH:
            keysym = XK_backslash;
            break;
        case AInput::TILDE:
            keysym = XK_grave;
            break;
        case AInput::SPACE:
            keysym = XK_space;
            break;
        case AInput::RETURN:
            keysym = XK_Return;
            break;
        case AInput::BACKSPACE:
            keysym = XK_BackSpace;
            break;
        case AInput::TAB:
            keysym = XK_Tab;
            break;
        case AInput::PAGEUP:
            keysym = XK_Prior;
            break;
        case AInput::PAGEDOWN:
            keysym = XK_Next;
            break;
        case AInput::END:
            keysym = XK_End;
            break;
        case AInput::HOME:
            keysym = XK_Home;
            break;
        case AInput::INSERT:
            keysym = XK_Insert;
            break;
        case AInput::DEL:
            keysym = XK_Delete;
            break;
        case AInput::ADD:
            keysym = XK_KP_Add;
            break;
        case AInput::SUBTRACT:
            keysym = XK_KP_Subtract;
            break;
        case AInput::MULTIPLY:
            keysym = XK_KP_Multiply;
            break;
        case AInput::DIVIDE:
            keysym = XK_KP_Divide;
            break;
        case AInput::PAUSE:
            keysym = XK_Pause;
            break;
        case AInput::F1:
            keysym = XK_F1;
            break;
        case AInput::F2:
            keysym = XK_F2;
            break;
        case AInput::F3:
            keysym = XK_F3;
            break;
        case AInput::F4:
            keysym = XK_F4;
            break;
        case AInput::F5:
            keysym = XK_F5;
            break;
        case AInput::F6:
            keysym = XK_F6;
            break;
        case AInput::F7:
            keysym = XK_F7;
            break;
        case AInput::F8:
            keysym = XK_F8;
            break;
        case AInput::F9:
            keysym = XK_F9;
            break;
        case AInput::F10:
            keysym = XK_F10;
            break;
        case AInput::F11:
            keysym = XK_F11;
            break;
        case AInput::F12:
            keysym = XK_F12;
            break;
        case AInput::F13:
            keysym = XK_F13;
            break;
        case AInput::F14:
            keysym = XK_F14;
            break;
        case AInput::F15:
            keysym = XK_F15;
            break;
        case AInput::LEFT:
            keysym = XK_Left;
            break;
        case AInput::RIGHT:
            keysym = XK_Right;
            break;
        case AInput::UP:
            keysym = XK_Up;
            break;
        case AInput::DOWN:
            keysym = XK_Down;
            break;
        case AInput::NUMPAD_0:
            keysym = XK_KP_Insert;
            break;
        case AInput::NUMPAD_1:
            keysym = XK_KP_End;
            break;
        case AInput::NUMPAD_2:
            keysym = XK_KP_Down;
            break;
        case AInput::NUMPAD_3:
            keysym = XK_KP_Page_Down;
            break;
        case AInput::NUMPAD_4:
            keysym = XK_KP_Left;
            break;
        case AInput::NUMPAD_5:
            keysym = XK_KP_Begin;
            break;
        case AInput::NUMPAD_6:
            keysym = XK_KP_Right;
            break;
        case AInput::NUMPAD_7:
            keysym = XK_KP_Home;
            break;
        case AInput::NUMPAD_8:
            keysym = XK_KP_Up;
            break;
        case AInput::NUMPAD_9:
            keysym = XK_KP_Page_Up;
            break;
        case AInput::A:
            keysym = XK_a;
            break;
        case AInput::B:
            keysym = XK_b;
            break;
        case AInput::C:
            keysym = XK_c;
            break;
        case AInput::D:
            keysym = XK_d;
            break;
        case AInput::E:
            keysym = XK_e;
            break;
        case AInput::F:
            keysym = XK_f;
            break;
        case AInput::G:
            keysym = XK_g;
            break;
        case AInput::H:
            keysym = XK_h;
            break;
        case AInput::I:
            keysym = XK_i;
            break;
        case AInput::J:
            keysym = XK_j;
            break;
        case AInput::K:
            keysym = XK_k;
            break;
        case AInput::L:
            keysym = XK_l;
            break;
        case AInput::M:
            keysym = XK_m;
            break;
        case AInput::N:
            keysym = XK_n;
            break;
        case AInput::O:
            keysym = XK_o;
            break;
        case AInput::P:
            keysym = XK_p;
            break;
        case AInput::Q:
            keysym = XK_q;
            break;
        case AInput::R:
            keysym = XK_r;
            break;
        case AInput::S:
            keysym = XK_s;
            break;
        case AInput::T:
            keysym = XK_t;
            break;
        case AInput::U:
            keysym = XK_u;
            break;
        case AInput::V:
            keysym = XK_v;
            break;
        case AInput::W:
            keysym = XK_w;
            break;
        case AInput::X:
            keysym = XK_x;
            break;
        case AInput::Y:
            keysym = XK_y;
            break;
        case AInput::Z:
            keysym = XK_z;
            break;
        case AInput::NUM0:
            keysym = XK_0;
            break;
        case AInput::NUM1:
            keysym = XK_1;
            break;
        case AInput::NUM2:
            keysym = XK_2;
            break;
        case AInput::NUM3:
            keysym = XK_3;
            break;
        case AInput::NUM4:
            keysym = XK_4;
            break;
        case AInput::NUM5:
            keysym = XK_5;
            break;
        case AInput::NUM6:
            keysym = XK_6;
            break;
        case AInput::NUM7:
            keysym = XK_7;
            break;
        case AInput::NUM8:
            keysym = XK_8;
            break;
        case AInput::NUM9:
            keysym = XK_9;
            break;
        default:
            keysym = 0;
            break;
    }
    return keysym;
}

bool PlatformAbstractionX11::inputIsKeyDown(AInput::Key k) {
    if (PlatformAbstractionX11::ourDisplay == nullptr)
        return false;
    if (k == AInput::Key::LBUTTON || k == AInput::Key::RBUTTON) {
        return isMouseKeyDown(k);
    }
    auto keysym = (KeySym) toNative(k);

    // Convert to keycode
    KeyCode keycode = XKeysymToKeycode(PlatformAbstractionX11::ourDisplay, keysym);
    if (keycode != 0) {
        // Get the whole keyboard state
        char keys[32];
        XQueryKeymap(PlatformAbstractionX11::ourDisplay, keys);

        // Check our keycode
        return (keys[keycode / 8] & (1 << (keycode % 8))) != 0;
    } else {
        return false;
    }
}
