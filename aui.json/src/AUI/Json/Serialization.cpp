// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

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
                    AString result = t.readStringUntilUnescaped('\"');
                    result.replaceAll("\\\\", "\\");
                    return result;
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