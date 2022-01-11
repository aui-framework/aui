//
// Created by alex2772 on 5/28/21.
//

#include <AUI/IO/AByteBufferOutputStream.h>
#include "IJsonElement.h"
#include "JsonException.h"

AString IJsonElement::formatMiscastException(const AString& message) {
    auto buf = _new<AByteBuffer>();
    serialize(AByteBufferOutputStream(buf));
    return message + ": " + AString::fromUtf8(*buf);
}
