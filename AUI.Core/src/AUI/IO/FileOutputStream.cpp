#include "FileOutputStream.h"


#include "FileNotFoundException.h"
#include "AUI/Common/AString.h"

FileOutputStream::FileOutputStream(const AString& path)
{
	mFile = fopen(path.toStdString().c_str(), "wb");
	if (!mFile)
	{
		throw FileNotFoundException(path.toStdString().c_str());
	}
}

FileOutputStream::~FileOutputStream()
{
	fclose(mFile);
}

int FileOutputStream::write(const char* dst, int size)
{
	return fwrite(dst, 1, size, mFile);
}
