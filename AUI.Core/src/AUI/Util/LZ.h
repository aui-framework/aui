#pragma once

#include "AUI/Core.h"
#include "AUI/Common/AException.h"

class ByteBuffer;

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
	void API_AUI_CORE compress(const ByteBuffer& b, ByteBuffer& dst);
	void API_AUI_CORE decompress(const ByteBuffer& b, ByteBuffer& dst);
}
