#pragma once

#include "AUI/Core.h"
#include "AUI/Common/AException.h"

class AByteBuffer;

class AZLibException: public AException
{
public:
	AZLibException()
	{
	}

	AZLibException(const AString& message)
		: AException(message)
	{
	}
};

namespace LZ {
	void API_AUI_CORE compress(const AByteBuffer& b, AByteBuffer& dst);
	void API_AUI_CORE decompress(const AByteBuffer& b, AByteBuffer& dst);
}
