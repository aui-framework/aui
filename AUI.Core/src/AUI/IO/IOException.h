#pragma once
#include <exception>

#include "AUI/Common/AException.h"

class IOException: public AException
{
public:
	IOException()
	{
	}

	IOException(const AString& message)
		: AException(message)
	{
	}
	virtual ~IOException() = default;
};
