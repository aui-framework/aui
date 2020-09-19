#pragma once
#include "IInputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Common/SharedPtr.h"

class API_AUI_CORE ByteBufferInputStream: public IInputStream
{
private:
	_<AByteBuffer> mBuffer;
	
public:
	ByteBufferInputStream(const _<AByteBuffer>& buffer)
		: mBuffer(buffer)
	{
	}

	int read(char* dst, int size) override;
};
