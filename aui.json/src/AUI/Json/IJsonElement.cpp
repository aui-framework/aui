//
// Created by alex2772 on 5/28/21.
//

#include "IJsonElement.h"
#include "JsonException.h"

AString IJsonElement::formatMiscastException(const AString& message) {
    AByteBuffer buf;
    serialize(buf);
    return message + ": " + AString::fromUtf8(buf);
}
