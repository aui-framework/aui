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

#include "Tokens.h"

AVector<token::Any> token::parse(aui::no_escape<ATokenizer> t) {
    AVector<token::Any> out;
    t->readChar();   // =
    try {
        while (!t->isEof()) {
            switch (char c = t->readChar()) {
                case ' ':
                    break;
                case '(':
                    out << token::LPar {};
                    break;
                case ')':
                    out << token::RPar {};
                    break;
                case ';':
                    out << token::Semicolon {};
                    break;
                case ':':
                    out << token::Colon {};
                    break;
                case '+':
                    out << token::Plus {};
                    break;
                case '-':
                    out << token::Minus {};
                    break;
                case '*':
                    out << token::Asterisk {};
                    break;
                case '/':
                    out << token::Slash {};
                    break;
                case '<':
                    out << token::LAngle {};
                    break;
                case '>':
                    out << token::RAngle {};
                    break;
                case '\'':
                    out << token::StringLiteral { t->readStringUntilUnescaped('\'') };
                    break;
                case '"':
                    out << token::StringLiteral { t->readStringUntilUnescaped('"') };
                    break;
                default:
                    if ('0' <= c && c <= '9') {
                        t->reverseByte();
                        out << token::Double { t->readFloat() };
                        continue;
                    }
                    if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z') {
                        t->reverseByte();
                        out << token::Identifier { t->readStringWhile([](char c) -> bool { return std::isalnum(c); }) };
                        continue;
                    }
                    throw AException("UNEXPECTED {}"_format(c));
            }
        }
    } catch (const AEOFException&) {}
    return out;
}
