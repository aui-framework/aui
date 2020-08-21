#pragma once
#include "AUI/Common/AVariant.h"
#include "AUI/Data.h"

class AString;

namespace AMeta
{
	API_AUI_DATA AVariant get(const AString& key);
    API_AUI_DATA void set(const AString& key, const AVariant& value);
}
