#pragma once
#include "AUI/Common/SharedPtr.h"
#include "AUI/IO/IInputStream.h"
#include "AUI/Json.h"
#include "AJsonElement.h"


namespace AJson
{
	AJsonElement API_AUI_JSON read(_<IInputStream> is);
}
