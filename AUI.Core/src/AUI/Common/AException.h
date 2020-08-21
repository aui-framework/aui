#pragma once
#include <exception>

#include "AString.h"

class API_AUI_CORE AException
{
private:
	AString mMessage;

public:
	AException()
	{
	}

	AException(const AString& message)
		: mMessage(message)
	{
	}

	virtual ~AException() noexcept;
	
	const AString& getMessage() const
	{
		return mMessage;
	}
};
