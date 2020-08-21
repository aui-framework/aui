#pragma once
#include "IInputStream.h"
#include "AUI/Common/ByteBuffer.h"
#include "AUI/Common/SharedPtr.h"

class API_AUI_CORE ByteBufferInputStream: public IInputStream
{
private:
	_<ByteBuffer> mBuffer;
	
public:
	ByteBufferInputStream(const _<ByteBuffer>& buffer)
		: mBuffer(buffer)
	{
	}

	int read(char* dst, int size) override;
};
