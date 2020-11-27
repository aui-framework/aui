#include "FileInputStream.h"


#include "FileNotFoundException.h"
#include "AUI/Common/AString.h"

FileInputStream::FileInputStream(const AString& path)
{
#ifdef _WIN32
    // КАК ЖЕ ЗАКОЛЕБАЛА ЭТА ВЕНДА
    _wfopen_s(&mFile, path.c_str(), L"rb");
#else
    mFile = fopen(path.toStdString().c_str(), "rb");
#endif
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
