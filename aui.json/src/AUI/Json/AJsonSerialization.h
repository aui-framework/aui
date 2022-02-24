#pragma once

#include <AUI/Json/AJsonElement.h>

namespace aui {
    template<>
    struct serializable<AJsonElement> {
        API_AUI_JSON static void write(IOutputStream& os, const AJsonElement& value);
        API_AUI_JSON static AJsonElement read(IInputStream& is);
    };
}