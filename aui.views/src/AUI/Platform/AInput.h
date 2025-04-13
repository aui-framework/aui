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

#pragma once

#include "AUI/Common/AString.h"
#include "AUI/Reflect/AEnumerate.h"

namespace AInput {
    enum Key {
        /* Based on SFML */
        UNKNOWN = -1, ///< Unhandled key
        LBUTTON = 0,      /// Left Mouse Button
        CBUTTON = 1,      /// Center Mouse Button
        RBUTTON = 2,      /// Right Mouse Button
        /* check APointerIndex for index values */

        ANDROID_BACK_BUTTON = 3, ///< The BACK key on android

        A = 'A',                 ///< The A key
        B,                       ///< The B key
        C,                       ///< The C key
        D,                       ///< The D key
        E,                       ///< The E key
        F,                       ///< The F key
        G,                       ///< The G key
        H,                       ///< The H key
        I,                       ///< The I key
        J,                       ///< The J key
        K,                       ///< The K key
        L,                       ///< The L key
        M,                       ///< The M key
        N,                       ///< The N key
        O,                       ///< The O key
        P,                       ///< The P key
        Q,                       ///< The Q key
        R,                       ///< The R key
        S,                       ///< The S key
        T,                       ///< The T key
        U,                       ///< The U key
        V,                       ///< The V key
        W,                       ///< The W key
        X,                       ///< The X key
        Y,                       ///< The Y key
        Z,                       ///< The Z key
        NUM0,                    ///< The 0 key
        NUM1,                    ///< The 1 key
        NUM2,                    ///< The 2 key
        NUM3,                    ///< The 3 key
        NUM4,                    ///< The 4 key
        NUM5,                    ///< The 5 key
        NUM6,                    ///< The 6 key
        NUM7,                    ///< The 7 key
        NUM8,                    ///< The 8 key
        NUM9,                    ///< The 9 key
        ESCAPE,                  ///< The Escape key
        LCONTROL,                ///< The left Control key
        LSHIFT,                  ///< The left Shift key
        LALT,                    ///< The left Alt key
        LSYSTEM,                 ///< The left OS specific key: window (Windows and Linux), apple (MacOS X), meta, super, gui i...
        RCONTROL,                ///< The right Control key
        RSHIFT,                  ///< The right Shift key
        RALT,                    ///< The right Alt key
        RSYSTEM,                 ///< The right OS specific key: window (Windows and Linux), apple (MacOS X), meta, super, gui i...
        MENU,                    ///< The Menu key
        LBRACKET,                ///< The [ key
        RBRACKET,                ///< The ] key
        SEMICOLON,               ///< The ; key
        COMMA,                   ///< The , key
        PERIOD,                  ///< The . key
        QUOTE,                   ///< The ' key
        SLASH,                   ///< The / key
        BACKSLASH,               ///< The \ key
        TILDE,                   ///< The ~ ` key
        EQUAL,                   ///< The = key
        DASH,                    ///< The - key
        SPACE,                   ///< The Space key
        RETURN,                  ///< The Return (Enter) key
        BACKSPACE,               ///< The Backspace key
        TAB,                     ///< The Tabulation key
        PAGEUP,                  ///< The Page up key
        PAGEDOWN,                ///< The Page down key
        END,                     ///< The End key
        HOME,                    ///< The Home key
        INSERT,                  ///< The Insert key
        DEL,                     ///< The Delete key
        ADD,                     ///< The + key
        SUBTRACT,                ///< The - key
        MULTIPLY,                ///< The * key
        DIVIDE,                  ///< The / key
        LEFT,                    ///< Left arrow
        RIGHT,                   ///< Right arrow
        UP,                      ///< Up arrow
        DOWN,                    ///< Down arrow
        F1,                      ///< The F1 key
        F2,                      ///< The F2 key
        F3,                      ///< The F3 key
        F4,                      ///< The F4 key
        F5,                      ///< The F5 key
        F6,                      ///< The F6 key
        F7,                      ///< The F7 key
        F8,                      ///< The F8 key
        F9,                      ///< The F9 key
        F10,                     ///< The F10 key
        F11,                     ///< The F11 key
        F12,                     ///< The F12 key
        F13,                     ///< The F13 key
        F14,                     ///< The F14 key
        F15,                     ///< The F15 key
        F16,                     ///< The F16 key
        F17,                     ///< The F17 key
        F18,                     ///< The F18 key
        F19,                     ///< The F19 key
        PAUSE,                   ///< The Pause key
        CAPSLOCK,                ///< The Capslock key
        SCROLLLOCK,              ///< The Scroll lock key
        NUMLOCKCLEAR,            ///< The NUMLOCK key on PC, CLEAR on mac
        PRINTSCREEN,             ///< The Printscreen key

        /* numpad */
        NUMPAD_0,                ///< The numpad 0 key
        NUMPAD_1,                ///< The numpad 1 key
        NUMPAD_2,                ///< The numpad 2 key
        NUMPAD_3,                ///< The numpad 3 key
        NUMPAD_4,                ///< The numpad 4 key
        NUMPAD_5,                ///< The numpad 5 key
        NUMPAD_6,                ///< The numpad 6 key
        NUMPAD_7,                ///< The numpad 7 key
        NUMPAD_8,                ///< The numpad 8 key
        NUMPAD_9,                ///< The numpad 9 key
        NUMPAD_ADD,              ///< The numpad + key
        NUMPAD_SUBTRACT,         ///< The numpad - key
        NUMPAD_MULTIPLY,         ///< The numpad * key
        NUMPAD_DIVIDE,           ///< The numpad / key
        NUMPAD_COMMA,            ///< The numpad , key
        NUMPAD_PERIOD,           ///< The numpad . key
        NUMPAD_EQUAL,            ///< The numpad = key
        NUMPAD_RETURN,           ///< The numpad Return (Enter) key

        /* media keys */
        VOLUMEUP,                ///< The volume up media key
        VOLUMEDOWN,              ///< The volume down media key
        MUTE,                    ///< The mute media key
        POWER,                   ///< The power button

        /* other */
        LANG1,                   ///< The Kana key (Cosmo_USB2ADB.c)
        LANG2,                   ///< The Eisu key (Cosmo_USB2ADB.c)
        INTERNATIONAL1,          ///< The Ro (JIS) key (Cosmo_USB2ADB.c)
        INTERNATIONAL3,          ///< The Yen (JIS) key (Cosmo_USB2ADB.c)

#if AUI_PLATFORM_MACOS
        CMD = LSYSTEM,           ///< The CMD key (Apple only)
#endif

        /* key count */
        KEYCOUNT,                ///< Keep last -- the total number of keyboard keys

        /* meta keys */
        CMD_CTRL = AUI_PLATFORM_APPLE ? CMD : LCONTROL, ///< Maps to CMD on Apple platforms, CTRL otherwise
    };
    API_AUI_VIEWS Key fromNative(int w);

    API_AUI_VIEWS int toNative(Key w);

    API_AUI_VIEWS AString getName(Key k);

    API_AUI_VIEWS bool isKeyDown(Key k);

    API_AUI_VIEWS void overrideStateForTesting(Key k, bool value);


    namespace native {
        API_AUI_VIEWS bool isKeyDown(Key k);
    }
};

AUI_ENUM_VALUES(AInput::Key,
                AInput::Key::UNKNOWN,
                AInput::Key::LBUTTON,
                AInput::Key::CBUTTON,
                AInput::Key::RBUTTON,
                AInput::Key::ANDROID_BACK_BUTTON,
                AInput::Key::A,
                AInput::Key::B,
                AInput::Key::C,
                AInput::Key::D,
                AInput::Key::E,
                AInput::Key::F,
                AInput::Key::G,
                AInput::Key::H,
                AInput::Key::I,
                AInput::Key::J,
                AInput::Key::K,
                AInput::Key::L,
                AInput::Key::M,
                AInput::Key::N,
                AInput::Key::O,
                AInput::Key::P,
                AInput::Key::Q,
                AInput::Key::R,
                AInput::Key::S,
                AInput::Key::T,
                AInput::Key::U,
                AInput::Key::V,
                AInput::Key::W,
                AInput::Key::X,
                AInput::Key::Y,
                AInput::Key::Z,
                AInput::Key::NUM0,
                AInput::Key::NUM1,
                AInput::Key::NUM2,
                AInput::Key::NUM3,
                AInput::Key::NUM4,
                AInput::Key::NUM5,
                AInput::Key::NUM6,
                AInput::Key::NUM7,
                AInput::Key::NUM8,
                AInput::Key::NUM9,
                AInput::Key::ESCAPE,
                AInput::Key::LCONTROL,
                AInput::Key::LSHIFT,
                AInput::Key::LALT,
                AInput::Key::LSYSTEM,
                AInput::Key::RCONTROL,
                AInput::Key::RSHIFT,
                AInput::Key::RALT,
                AInput::Key::RSYSTEM,
                AInput::Key::MENU,
                AInput::Key::LBRACKET,
                AInput::Key::RBRACKET,
                AInput::Key::SEMICOLON,
                AInput::Key::COMMA,
                AInput::Key::PERIOD,
                AInput::Key::QUOTE,
                AInput::Key::SLASH,
                AInput::Key::BACKSLASH,
                AInput::Key::TILDE,
                AInput::Key::EQUAL,
                AInput::Key::DASH,
                AInput::Key::SPACE,
                AInput::Key::RETURN,
                AInput::Key::BACKSPACE,
                AInput::Key::TAB,
                AInput::Key::PAGEUP,
                AInput::Key::PAGEDOWN,
                AInput::Key::END,
                AInput::Key::HOME,
                AInput::Key::INSERT,
                AInput::Key::DEL,
                AInput::Key::ADD,
                AInput::Key::SUBTRACT,
                AInput::Key::MULTIPLY,
                AInput::Key::DIVIDE,
                AInput::Key::LEFT,
                AInput::Key::RIGHT,
                AInput::Key::UP,
                AInput::Key::DOWN,
                AInput::Key::NUMPAD_0,
                AInput::Key::NUMPAD_1,
                AInput::Key::NUMPAD_2,
                AInput::Key::NUMPAD_3,
                AInput::Key::NUMPAD_4,
                AInput::Key::NUMPAD_5,
                AInput::Key::NUMPAD_6,
                AInput::Key::NUMPAD_7,
                AInput::Key::NUMPAD_8,
                AInput::Key::NUMPAD_9,
                AInput::Key::F1,
                AInput::Key::F2,
                AInput::Key::F3,
                AInput::Key::F4,
                AInput::Key::F5,
                AInput::Key::F6,
                AInput::Key::F7,
                AInput::Key::F8,
                AInput::Key::F9,
                AInput::Key::F10,
                AInput::Key::F11,
                AInput::Key::F12,
                AInput::Key::F13,
                AInput::Key::F14,
                AInput::Key::F15,
                AInput::Key::PAUSE,
                AInput::Key::KEYCOUNT)
