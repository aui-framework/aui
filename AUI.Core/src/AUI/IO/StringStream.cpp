#include "StringStream.h"

StringStream::StringStream(const AString& string): mString(string), mIterator(mString.begin())
{
}

int StringStream::read(char* dst, int size)
{
	if (mIterator == mString.end())
		return 0;

	int r = 0;
	for (; mIterator != mString.end() && r < size; ++r, ++mIterator)
	{
		dst[r] = char(*mIterator);
	}
	return r;
}
