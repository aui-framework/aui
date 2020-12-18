#include "StringStream.h"

StringStream::StringStream(const AString& string): mString(string.toStdString()), mIterator(mString.begin())
{
}

int StringStream::read(char* dst, int size)
{
	if (mIterator == mString.end())
		return 0;

	int toRead = glm::min(size, mString.end() - mIterator);
	memcpy(dst, &(*mIterator), toRead);
    mIterator += toRead;
	return toRead;
}
