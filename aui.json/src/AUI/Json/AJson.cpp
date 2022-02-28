#include "AJson.h"
#include "AUI/IO/AStringStream.h"
#include "AUI/Common/AByteBuffer.h"

AString AJson::toString(const AJson& json) {
    AByteBuffer buffer;
    aui::serialize(buffer, json);
    return AString::fromUtf8(buffer);
}

AJson AJson::fromString(const AString& json) {
    AStringStream sis(json);
    return aui::deserialize<AJson>(sis);
}
