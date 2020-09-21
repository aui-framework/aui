#pragma once

#include <AUI/IO/IOutputStream.h>
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/Json.h"
#include "AJsonElement.h"


namespace AJson
{
    API_AUI_JSON AJsonElement read(_<IInputStream> is);
    API_AUI_JSON void write(_<IOutputStream> os, const AJsonElement& json);

    API_AUI_JSON AString toString(const AJsonElement& json);
    API_AUI_JSON AJsonElement fromString(const AString& json);
}
