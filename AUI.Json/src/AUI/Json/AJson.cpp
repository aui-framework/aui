#include <AUI/IO/ByteBufferOutputStream.h>
#include <AUI/IO/ByteBufferInputStream.h>
#include "AJson.h"
#include "AUI/Util/ATokenizer.h"
#include "JsonArray.h"
#include "JsonValue.h"
#include "JsonObject.h"
#include "JsonNull.h"
#include "JsonException.h"
#include "AJsonElement.h"


AJsonElement AJson::read(_<IInputStream> is) {
    ATokenizer t(is);
    try {

        std::function < _<IJsonElement>() > read;

        auto unexpectedCharacter = [&]() {
            throw JsonException(
                    AString("unexpected character ") + t.getLastCharacter() + " at " + AString::number(t.getRow()) + ":"
                    + AString::number(t.getColumn()));
        };

        read = [&]() -> _<IJsonElement> {
            _<IJsonElement> result;

            for (;;) {
                switch (t.readChar()) {
                    case '[':
                        result = _new<JsonArray>();
                        while (t.readChar() != ']') {
                            t.reverseByte();
                            result->asArray() << AJsonElement(read());
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
                                    result->asObject()[key] = AJsonElement(read());
                                } else if (t.getLastCharacter() != ',') {
                                    unexpectedCharacter();
                                }
                            }
                        }

                        return result;

                    case '\"':
                        result = _new<JsonValue>(t.readStringUntilUnescaped('\"'));
                        return result;
                }

                if (isdigit(t.getLastCharacter())) {
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
        };

        return AJsonElement(read());
    } catch (const AException& e) {
        throw AException(AString::number(t.getRow()) + ":" + AString::number(t.getColumn()) + ": " + e.getMessage());
    }
    return AJsonElement();
}

void API_AUI_JSON AJson::write(_<IOutputStream> os, const AJsonElement& json) {
    json.serialize(os);
}

AJsonElement::AJsonElement(std::nullptr_t) :
        mJson(_new<JsonNull>()) {

}

bool AJsonElement::isNull() const {
    return mJson->isNull();
}

AString AJson::toString(const AJsonElement& json) {
    auto bb = _new<AByteBuffer>();
    write(_new<ByteBufferOutputStream>(bb), json);
    return {bb->data(), bb->data() + bb->getSize()};
}

AJsonElement AJson::fromString(const AString& json) {
    auto bb = _new<AByteBuffer>();
    auto s = json.toStdString();
    bb->put(s.c_str(), s.length());
    bb->setCurrentPos(0);
    return read(_new<ByteBufferInputStream>(bb));
}
