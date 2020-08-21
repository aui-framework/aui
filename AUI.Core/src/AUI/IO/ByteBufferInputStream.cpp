#include "ByteBufferInputStream.h"
#include <glm/glm.hpp>

int ByteBufferInputStream::read(char* dst, int size)
{
	const int toRead = glm::min(mBuffer->getAvailable(), static_cast<size_t>(size));
	if (toRead)
	{
		mBuffer->get(dst, toRead);
		return toRead;
	}
	return 0;
}
