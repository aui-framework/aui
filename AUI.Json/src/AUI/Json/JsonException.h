#pragma once
#include "AUI/Common/AException.h"
#include <AUI/Json.h>

class API_AUI_JSON JsonException: public AException
{
public:

	JsonException() = default;

	explicit JsonException(const AString& message)
		: AException(message)
	{
	}
};
