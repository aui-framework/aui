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

#include <AUI/IO/AByteBufferInputStream.h>
#include "AJson.h"
#include "AUI/Util/ATokenizer.h"
#include "JsonArray.h"
#include "JsonValue.h"
#include "JsonObject.h"
#include "JsonNull.h"
#include "JsonException.h"
#include "AJsonElement.h"


static _<IJsonElement> read(ATokenizer& t) {
    _<IJsonElement> result;

    auto unexpectedCharacter = [&]() {
        throw JsonException(
                AString("unexpected character ") + t.getLastCharacter() + " at " + AString::number(t.getRow()) + ":"
                + AString::number(t.getColumn()));
    };
    auto unexpectedToken = [&](const AString& token) {
        throw JsonException(
                AString("unexpected token ") + token + " at " + AString::number(t.getRow()) + ":"
                + AString::number(t.getColumn()));
    };
    for (;;) {
        switch (t.readChar()) {
            case '[':
                result = _new<JsonArray>();
                while (t.readChar() != ']') {
                    t.reverseByte();
                    result->asArray() << AJsonElement(read(t));
                    char c = t.readChar();
                    for (; c != ',' && c != ']'; c = t.readChar()) {
                        if (!isspace(c))
                            unexpectedCharacter();
                    }
                    if (c == ']')
                        break;
                }
                return result;
            case '{':
                result = _new<JsonObject>();
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
                            result->asObject()[key] = AJsonElement(read(t));
                        } else if (t.getLastCharacter() != ',') {
                            unexpectedCharacter();
                        }
                    }
                }

                return result;

            case 't': // true?
            {
                t.reverseByte();
                auto s = t.readString();
                if (s == "true") {
                    result = _new<JsonValue>(true);
                    return result;
                }
                unexpectedToken(s);
            }

            case 'f': // false?
            {
                t.reverseByte();
                auto s = t.readString();
                if (s == "false") {
                    result = _new<JsonValue>(false);
                    return result;
                }
                unexpectedToken(s);
            }

            case '\"':
                result = _new<JsonValue>(t.readStringUntilUnescaped('\"'));
                return result;
        }

        if (isdigit(uint8_t(t.getLastCharacter()))) {
            t.reverseByte();
            return _new<JsonValue>(t.readInt());
        }

        t.reverseByte();
        AString keyword = t.readString();
        if (keyword == "null") {
            return _new<JsonNull>();
        }
        t.readChar();
    }
    assert(!isspace(t.getLastCharacter()));
    return result;
}

AJsonElement AJson::read(IInputStream& is) {
    ATokenizer t(aui::ptr::fake(&is));
    try {
        return AJsonElement(read(t));
    } catch (const AException& e) {
        throw AException(AString::number(t.getRow()) + ":" + AString::number(t.getColumn()) + ": " + e.getMessage());
    }
    return AJsonElement();
}

void API_AUI_JSON AJson::write(IOutputStream& os, const AJsonElement& json) {
    json.serialize(os);
}


AString AJson::toString(const AJsonElement& json) {
    auto bb = _new<AByteBuffer>();
    write(bb, json);
    return {bb->data(), bb->data() + bb->getSize()};
}

AJsonElement AJson::fromString(const AString& json) {
    AByteBuffer bb;
    auto s = json.toStdString();
    bb.write(s.c_str(), s.length());
    bb.setCurrentPos(0);
    return read(AByteBufferInputStream(bb));
}
