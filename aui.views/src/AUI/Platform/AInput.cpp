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
