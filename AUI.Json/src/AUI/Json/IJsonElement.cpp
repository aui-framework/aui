//
// Created by alex2772 on 5/28/21.
//

#include <AUI/IO/ByteBufferOutputStream.h>
#include "IJsonElement.h"
#include "JsonException.h"

void IJsonElement::throwMiscastException(const AString& message) {
    auto buf = _new<AByteBuffer>();
    serialize(_new<ByteBufferOutputStream>(buf));
    throw JsonException(message + ": " + AString::fromUtf8(*buf));
}
