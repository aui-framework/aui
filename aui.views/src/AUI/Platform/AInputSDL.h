//
// Created by alex2 on 6/11/2021.
//

#pragma once

/**
 * SDL <=> AUI extensions
 */
 
#include <SDL.h>
#include <AUI/Platform/AInput.h>

namespace AInput {
    static AInput::Key fromSDL(SDL_Keycode keycode) {
        AInput::Key vkey;
        switch (keycode) {
            default:           vkey = AInput::Unknown;     break;
			case 'A':case 'a': vkey = AInput::A;           break;
            case 'B':case 'b': vkey = AInput::B;           break;
            case 'C':case 'c': vkey = AInput::C;           break;
            case 'D':case 'd': vkey = AInput::D;           break;
            case 'E':case 'e': vkey = AInput::E;           break;
            case 'F':case 'f': vkey = AInput::F;           break;
            case 'G':case 'g': vkey = AInput::G;           break;
            case 'H':case 'h': vkey = AInput::H;           break;
            case 'I':case 'i': vkey = AInput::I;           break;
            case 'J':case 'j': vkey = AInput::J;           break;
            case 'K':case 'k': vkey = AInput::K;           break;
            case 'L':case 'l': vkey = AInput::L;           break;
            case 'M':case 'm': vkey = AInput::M;           break;
            case 'N':case 'n': vkey = AInput::N;           break;
            case 'O':case 'o': vkey = AInput::O;           break;
            case 'P':case 'p': vkey = AInput::P;           break;
            case 'Q':case 'q': vkey = AInput::Q;           break;
            case 'R':case 'r': vkey = AInput::R;           break;
            case 'S':case 's': vkey = AInput::S;           break;
            case 'T':case 't': vkey = AInput::T;           break;
            case 'U':case 'u': vkey = AInput::U;           break;
            case 'V':case 'v': vkey = AInput::V;           break;
            case 'W':case 'w': vkey = AInput::W;           break;
            case 'X':case 'x': vkey = AInput::X;           break;
            case 'Y':case 'y': vkey = AInput::Y;           break;
            case 'Z':case 'z': vkey = AInput::Z;           break;
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
            case SDLK_ESCAPE:     vkey = AInput::Escape;     break;
            case SDLK_LCTRL:   vkey = AInput::LControl;   break;
            case SDLK_LSHIFT:     vkey = AInput::LShift;     break;
            case SDLK_LALT:       vkey = AInput::LAlt;      break;
            case SDLK_LGUI:    vkey = AInput::LSystem;       break;
            case SDLK_RCTRL:   vkey = AInput::RControl;   break;
            case SDLK_RSHIFT:     vkey = AInput::RShift;     break;
            case SDLK_RALT:       vkey = AInput::RAlt;      break;
            case SDLK_RGUI:    vkey = AInput::RSystem;       break;
            case SDLK_MENU:       vkey = AInput::Menu;       break;
            case SDLK_LEFTBRACKET:   vkey = AInput::LBracket;      break;
            case SDLK_RIGHTBRACKET:   vkey = AInput::RBracket;      break;
            case SDLK_SEMICOLON:  vkey = AInput::SemiColon;      break;
            case SDLK_COMMA:      vkey = AInput::Comma;  break;
            case SDLK_PERIOD:     vkey = AInput::Period; break;
            case SDLK_QUOTE:      vkey = AInput::Quote;      break;
            case SDLK_SLASH:      vkey = AInput::Slash;      break;
            case SDLK_BACKSLASH:  vkey = AInput::BackSlash;      break;
            case SDLK_BACKQUOTE:      vkey = AInput::Tilde;      break;
            case SDLK_PLUS:      vkey = AInput::Equal;   break;
            case SDLK_MINUS:       vkey = AInput::Dash;  break;
            case SDLK_SPACE:      vkey = AInput::Space;      break;
            case SDLK_RETURN:     vkey = AInput::Return;     break;
            case SDLK_BACKSPACE:  vkey = AInput::BackSpace;       break;
            case SDLK_TAB:        vkey = AInput::Tab;        break;
            case SDLK_PAGEUP:     vkey = AInput::PageUp;      break;
            case SDLK_PAGEDOWN:   vkey = AInput::PageDown;       break;
            case SDLK_END:        vkey = AInput::End;        break;
            case SDLK_HOME:       vkey = AInput::Home;       break;
            case SDLK_INSERT:     vkey = AInput::Insert;     break;
            case SDLK_DELETE:     vkey = AInput::Delete;     break;
            case SDLK_LEFT:       vkey = AInput::Left;       break;
            case SDLK_RIGHT:      vkey = AInput::Right;      break;
            case SDLK_UP:         vkey = AInput::Up;         break;
            case SDLK_DOWN:       vkey = AInput::Down;       break;
            case SDLK_KP_0:    vkey = AInput::Numpad0;    break;
            case SDLK_KP_1:    vkey = AInput::Numpad1;    break;
            case SDLK_KP_2:    vkey = AInput::Numpad2;    break;
            case SDLK_KP_3:    vkey = AInput::Numpad3;    break;
            case SDLK_KP_4:    vkey = AInput::Numpad4;    break;
            case SDLK_KP_5:    vkey = AInput::Numpad5;    break;
            case SDLK_KP_6:    vkey = AInput::Numpad6;    break;
            case SDLK_KP_7:    vkey = AInput::Numpad7;    break;
            case SDLK_KP_8:    vkey = AInput::Numpad8;    break;
            case SDLK_KP_9:    vkey = AInput::Numpad9;    break;
            case SDLK_F1:         vkey = AInput::F1;         break;
            case SDLK_F2:         vkey = AInput::F2;         break;
            case SDLK_F3:         vkey = AInput::F3;         break;
            case SDLK_F4:         vkey = AInput::F4;         break;
            case SDLK_F5:         vkey = AInput::F5;         break;
            case SDLK_F6:         vkey = AInput::F6;         break;
            case SDLK_F7:         vkey = AInput::F7;         break;
            case SDLK_F8:         vkey = AInput::F8;         break;
            case SDLK_F9:         vkey = AInput::F9;         break;
            case SDLK_F10:        vkey = AInput::F10;        break;
            case SDLK_F11:        vkey = AInput::F11;        break;
            case SDLK_F12:        vkey = AInput::F12;        break;
            case SDLK_F13:        vkey = AInput::F13;        break;
            case SDLK_F14:        vkey = AInput::F14;        break;
            case SDLK_F15:        vkey = AInput::F15;        break;
            case SDLK_PAUSE:      vkey = AInput::Pause;      break;
        }
        return vkey;
    }
}