#pragma once

#include <AUI/Traits/serializable.h>
#include "AJson.h"

template<>
struct ASerializable<AJson> {
    API_AUI_JSON static void write(IOutputStream& os, const AJson& value);
    API_AUI_JSON static void read(IInputStream& is, AJson& dst);
};