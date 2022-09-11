/*
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

#include <AUI/IO/AByteBufferInputStream.h>
#include "AJson.h"
#include "AUI/Util/ATokenizer.h"
#include "AJson.h"
#include "Serialization.h"
#include "AUI/Traits/callables.h"


static AJson read(ATokenizer& t) {
    try {
        auto unexpectedCharacter = [&]() {
            throw AJsonParseException(
                    AString("unexpected character ") + t.getLastCharacter() + " at " + AString::number(t.getRow()) + ":"
                    + AString::number(t.getColumn()));
        };
        auto unexpectedToken = [&](const AString& token) {
            throw AJsonParseException(
                    AString("unexpected token ") + token + " at " + AString::number(t.getRow()) + ":"
                    + AString::number(t.getColumn()));
        };

        for (;;) {
            switch (t.readChar()) {
                case '[': {
                    aui::impl::JsonArray result;
                    while (t.readChar() != ']') {
                        t.reverseByte();
                        result << read(t);
                        char c = t.readChar();
                        for (; c != ',' && c != ']'; c = t.readChar()) {
                            if (!isspace(c))
                                unexpectedCharacter();
                        }
                        if (c == ']')
                            break;
                    }
                    return std::move(result);
                }
                case '{': {
                    aui::impl::JsonObject result;
                    while (t.readChar() != '}') {
                        if (!isspace(t.getLastCharacter())) {
                            if (t.getLastCharacter() == '\"') {
                                AString key = t.readStringUntilUnescaped('\"');

                                for (char c = 0;;) {
                                    c = t.readChar();
                                    if (c == ':')
                                        break;
                                    if (!isspace(c))
                                        unexpectedCharacter();
                                }
                                result[key] = read(t);
                            } else if (t.getLastCharacter() != ',') {
                                unexpectedCharacter();
                            }
                        }
                    }

                    return result;
                }
                case 't': // true?
                {
                    t.reverseByte();
                    auto s = t.readString();
                    if (s == "true") {
                        return true;
                    }
                    unexpectedToken(s);
                }

                case 'f': // false?
                {
                    t.reverseByte();
                    auto s = t.readString();
                    if (s == "false") {
                        return false;
                    }
                    unexpectedToken(s);
                }

                case '\"':
                    return t.readStringUntilUnescaped('\"').replacedAll("\\\\", "\\");
            }

            if (isdigit(uint8_t(t.getLastCharacter())) || t.getLastCharacter() == '-') {
                t.reverseByte();
                auto longInt = t.readLongInt();
                int basicInt = longInt;
                if (longInt == basicInt) {
                    return basicInt;
                }
                return longInt;
            }

            t.reverseByte();
            AString keyword = t.readString();
            if (keyword == "null") {
                return nullptr;
            }
            if (!keyword.empty()) throw AJsonParseException("invalid keyword: {}"_format(keyword));

            if (char c = t.readChar(); !isspace(c)) {
                throw AJsonParseException("invalid char: {}"_format(c));
            }
        }
    } catch (const AEOFException& e) {
        throw AJsonParseException("unexpected end of json stream");
    }
    throw AJsonParseException("internal parser error");
}


void ASerializable<AJson>::write(IOutputStream& os, const AJson& value) {
    std::visit(aui::lambda_overloaded {
        [&](int v) {
            os << AString::number(v);
        },
        [&](int64_t v) {
            os << AString::number(v);
        },
        [&](double v) {
            os << AString::number(v);
        },
        [&](bool v) {
            os << (v ? "true" : "false");
        },
        [&](const AString& v) {
            os << '"' << v.replacedAll("\\", "\\\\").replacedAll("\"", "\\\"") << '"';
        },
        [&](std::nullptr_t) {
            os << "null";
        },
        [&](const aui::impl::JsonArray& v) {
            os << '[';
            for (auto it = v.begin(); it != v.end(); ++it) {
                if (it != v.begin()) {
                    os << ',';
                }
                os << *it;
            }
            os << ']';
        },
        [&](const aui::impl::JsonObject & v) {
            os << '{';
            for (auto it = v.begin(); it != v.end(); ++it) {
                if (it != v.begin()) {
                    os << ',';
                }
                os << '"' << it->first.replacedAll("\"", "\\\"") << "\":" << it->second;
            }
            os << '}';
        },
        [&](std::nullopt_t) {
            // empty value
        },
    }, static_cast<const aui::impl::JsonVariant&>(value));
}

void ASerializable<AJson>::read(IInputStream& is, AJson& dst) {
    ATokenizer t(aui::ptr::fake(&is));
    dst = ::read(t);
}