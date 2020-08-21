#include "FileInputStream.h"


#include "FileNotFoundException.h"
#include "AUI/Common/AString.h"

FileInputStream::FileInputStream(const AString& path)
{
	mFile = fopen(path.toStdString().c_str(), "rb");
	if (!mFile)
	{
		throw FileNotFoundException(path.toStdString().c_str());
	}
}

FileInputStream::~FileInputStream()
{
	fclose(mFile);
}

int FileInputStream::read(char* dst, int size)
{
	size_t r = ::fread(dst, 1, size, mFile);
	return r;
}
