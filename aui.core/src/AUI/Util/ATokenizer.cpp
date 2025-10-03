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
                        char32_t codepoint = 0;
                        int hexDigitsRead = 0;

                        auto parseHexDigit = [](char c) -> int {
                            if (c >= '0' && c <= '9') return c - '0';
                            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                            return -1;
                        };

                        for (int i = 0; i < 4; ++i) {
                            char hexChar = readChar();
                            int hexValue = parseHexDigit(hexChar);

                            if (hexValue == -1) {
                                reverseByte();
                                break;
                            }

                            codepoint = (codepoint << 4) | hexValue;
                            hexDigitsRead++;
                        }

                        if (hexDigitsRead == 4) {
                            if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
                                char next1 = readChar();
                                char next2 = readChar();

                                if (next1 == '\\' && next2 == 'u') {
                                    char32_t lowSurrogate = 0;
                                    int lowHexDigitsRead = 0;

                                    for (int i = 0; i < 4; ++i) {
                                        char hexChar = readChar();
                                        int hexValue = parseHexDigit(hexChar);

                                        if (hexValue == -1) {
                                            reverseByte();
                                            break;
                                        }

                                        lowSurrogate = (lowSurrogate << 4) | hexValue;
                                        lowHexDigitsRead++;
                                    }

                                    if (lowHexDigitsRead == 4 && lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF) {
                                        codepoint = 0x10000 + ((codepoint & 0x3FF) << 10) + (lowSurrogate & 0x3FF);
                                    } else {
                                        for (int i = 0; i < lowHexDigitsRead; ++i) {
                                            reverseByte();
                                        }
                                        reverseByte(); // 'u'
                                        reverseByte(); // '\'
                                    }
                                } else {
                                    reverseByte(); // next2
                                    reverseByte(); // next1
                                }
                            }

                            if ((codepoint <= 0x10FFFF) &&
                                !(codepoint >= 0xD800 && codepoint <= 0xDFFF) &&
                                codepoint != 0xFFFE && codepoint != 0xFFFF) {

                                if (codepoint <= 0x7F) {
                                    //= 0xxxxxxx
                                    out += static_cast<char>(codepoint);
                                } else if (codepoint <= 0x7FF) {
                                    // 110xxxxx 10xxxxxx
                                    out += static_cast<char>(0xC0 | (codepoint >> 6));
                                    out += static_cast<char>(0x80 | (codepoint & 0x3F));
                                } else if (codepoint <= 0xFFFF) {
                                    // 1110xxxx 10xxxxxx 10xxxxxx
                                    out += static_cast<char>(0xE0 | (codepoint >> 12));
                                    out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                                    out += static_cast<char>(0x80 | (codepoint & 0x3F));
                                } else {
                                    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                                    out += static_cast<char>(0xF0 | (codepoint >> 18));
                                    out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                                    out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                                    out += static_cast<char>(0x80 | (codepoint & 0x3F));
                                }
                            } else {
                                out += "\xEF\xBF\xBD"; // U+FFFD
                            }
                        } else {
                            out += "\\u";
                            for (int i = 0; i < hexDigitsRead; ++i) {
                                reverseByte();
                            }
                        }
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
