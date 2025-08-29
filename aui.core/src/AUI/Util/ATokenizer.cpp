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

#include "ATokenizer.h"
#include "AUI/Common/ADynamicVector.h"
#include "AUI/Common/AOptional.h"
#include "AUI/Common/AStaticVector.h"
#include "AUI/Common/AStringVector.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/ASet.h"
#include "AUI/IO/AStringStream.h"

ATokenizer::ATokenizer(const AString& fromString):
    mInput(_new<AStringStream>(fromString))
{
}

const std::string& ATokenizer::readString()
{
    mTemporaryStringBuffer.clear();
    char c = 0;

    try
    {
        for (;;) {
            c = readChar();
            if (isalnum(uint8_t (c)))
            {
                mTemporaryStringBuffer.push_back(c);
            }
            else
            {
                reverseByte();
                return mTemporaryStringBuffer;
            }
        }
    } catch (...)
    {
        mEof = true;
    }
    return mTemporaryStringBuffer;
}
const std::string& ATokenizer::readString(const ASet<char>& applicableChars)
{
    mTemporaryStringBuffer.clear();
    char c = 0;

    try
    {
        for (;;) {
            c = readChar();
            if (isalnum(uint8_t (c)) || applicableChars.find(c) != applicableChars.end())
            {
                mTemporaryStringBuffer.push_back(c);
            }
            else
            {
                reverseByte();
                return mTemporaryStringBuffer;
            }
        }
    } catch (...)
    {
        mEof = true;
    }
    return mTemporaryStringBuffer;
}


void ATokenizer::reverseByte()
{
    mReverse = true;
}

float ATokenizer::readFloat()
{
    mTemporaryAStringBuffer.clear();
    try {
        bool dot = false;
        char c = 0;
        for (;;)
        {
            c = readChar();
            switch (c)
            {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-':
                mTemporaryAStringBuffer << c;
                break;
            case '.':
                if (!dot)
                {
                    mTemporaryAStringBuffer << c;
                    break;
                }
            default:
                reverseByte();
                return mTemporaryAStringBuffer.toFloat().valueOr(0);
            }
        }
    } catch (...) {
        mEof = true;
    }
    return mTemporaryAStringBuffer.toFloat().valueOr(0);
}

template<typename T>
T ATokenizer::readIntImpl() {
    static_assert(std::is_integral_v<T>, "readIntImpl accepts only integral type");
    mTemporaryAStringBuffer.clear();
    auto value = [&] {
        if constexpr(sizeof(T) > 4) {
            return mTemporaryAStringBuffer.toLong().valueOr(0);
        } else {
            return mTemporaryAStringBuffer.toInt().valueOr(0);
        }
    };
    try {
        char c = 0;
        for (;;)
        {
            c = readChar();
            switch (c)
            {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case 'x':
                case 'X':

                    // hex
                case 'a':
                case 'A':
                case 'b':
                case 'B':
                case 'c':
                case 'C':
                case 'd':
                case 'D':
                case 'e':
                case 'E':
                case 'f':
                case 'F':
                    mTemporaryAStringBuffer << c;
                    break;
                case '-':
                    if (mTemporaryAStringBuffer.empty()) {
                        mTemporaryAStringBuffer << c;
                        break;
                    }
                    [[fallthrough]];
                default:
                    reverseByte();
                    return value();
            }
        }
    }
    catch (...) {
        mEof = true;
    }
    return value();
}

ATokenizer::Hexable<unsigned> ATokenizer::readUIntX() {
    mTemporaryAStringBuffer.clear();
    bool isHex = false;
    try {
        char c = 0;
        for (;;)
        {
            c = readChar();
            switch (c)
            {
                case 'x':
                case 'X':
                    isHex = true;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    mTemporaryAStringBuffer << c;
                    break;

                    // hex
                case 'a':
                case 'A':
                case 'b':
                case 'B':
                case 'c':
                case 'C':
                case 'd':
                case 'D':
                case 'e':
                case 'E':
                case 'f':
                case 'F':
                    if (isHex) {
                        mTemporaryAStringBuffer << c;
                        break;
                    }
                default:
                    reverseByte();
                    return {.value=mTemporaryAStringBuffer.toUInt().valueOr(0), .isHex=isHex};
            }
        }
    }
    catch (...) {
        mEof = true;
    }
    return {.value=mTemporaryAStringBuffer.toUInt().valueOr(0), .isHex=isHex};
}

void ATokenizer::skipUntilUnescaped(char c) {
    for (char current = 0; (current = readChar()) != c;)
    {
        if (current == '\\')
        {
            readChar();
        }
    }
}

const std::string& ATokenizer::readStringUntilUnescaped(char c)
{
    mTemporaryStringBuffer.clear();
    readStringUntilUnescaped(mTemporaryStringBuffer, c);
    return mTemporaryStringBuffer;
}
const std::string& ATokenizer::readStringUntilUnescaped(const ASet<char>& characters) {
    mTemporaryStringBuffer.clear();
    readStringUntilUnescaped(mTemporaryStringBuffer, characters);
    return mTemporaryStringBuffer;
}

void ATokenizer::readStringUntilUnescaped(std::string& out, char c)
{
    try {
        for (char current = 0; (current = readChar()) != c;)
        {
            if (current == '\\')
            {
                char tmp = readChar();
                switch (tmp) {
                    case 'r': out += '\r'; break;
                    case 'n': out += '\n'; break;
                    case 't': out += '\t'; break;
                    case 'u': {
                        // utf8 sequence
                        AString::value_type currentChar = 0;
                        auto commit4bitValue = [&](uint8_t v) {
                            currentChar = currentChar << 4 | v;
                        };
                        for (;;) {
                            char bit4 = readChar();
                            if (bit4 >= '0' && bit4 <= '9') {
                                commit4bitValue(bit4 - '0');
                                continue;
                            }

                            if (bit4 >= 'a' && bit4 <= 'f') {
                                static constexpr auto BASE = 10;
                                commit4bitValue(bit4 - 'a' + BASE);
                                continue;
                            }

                            if (bit4 >= 'A' && bit4 <= 'F') {
                                commit4bitValue(bit4 - 'A');
                                continue;
                            }
                            reverseByte();
                            break;
                        }
                        out += AString(1, currentChar).toStdString();

                        break;
                    }
                    default: out += tmp;
                }
            }
            else
            {
                out += current;
            }
        }
    }
    catch (...) {
        mEof = true;
    }
}

void ATokenizer::readStringUntilUnescaped(std::string& out, const ASet<char>& characters) {
    try {
        for (char current = 0; !characters.contains(current = readChar());)
        {
            if (current == '\\')
            {
                out += '\\';
                out += readChar();
            }
            else
            {
                out += current;
            }
        }
    }
    catch (...) {
        mEof = true;
    }
}

glm::vec2 ATokenizer::readVec2()
{
    glm::vec2 result;
    result.x = readFloat();
    readChar();
    result.y = readFloat();
    return result;
}

const std::string& ATokenizer::readString(size_t n) {
    mTemporaryStringBuffer.clear();
    mTemporaryStringBuffer.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        mTemporaryStringBuffer += readChar();
    }
    return mTemporaryStringBuffer;
}

void ATokenizer::skipUntil(char c) {
    for (char x; (x = readChar()) != c; );
}

unsigned ATokenizer::readUInt() {
    return readIntImpl<unsigned>();
}

int ATokenizer::readInt() {
    return readIntImpl<int>();
}

int64_t ATokenizer::readLongInt() {
    return readIntImpl<int64_t>();
}
