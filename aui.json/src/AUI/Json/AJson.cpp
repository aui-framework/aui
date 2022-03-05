#include "AJson.h"
#include "AUI/IO/AStringStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/IO/AByteBufferInputStream.h"

AString AJson::toString(const AJson& json) {
    AByteBuffer buffer;
    aui::serialize(buffer, json);
    return AString::fromBuffer(buffer);
}

AJson AJson::fromString(AString json) {
    AStringStream sis(std::move(json));
    return aui::deserialize<AJson>(sis);
}

AJson AJson::fromBuffer(AByteBufferView buffer) {
    return aui::deserialize<AJson>(AByteBufferInputStream(buffer));
}
