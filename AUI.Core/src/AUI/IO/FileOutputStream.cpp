#include "FileOutputStream.h"


#include "FileNotFoundException.h"
#include "AUI/Common/AString.h"

FileOutputStream::FileOutputStream(const AString& path, bool append)
{
#ifdef _WIN32
	// КАК ЖЕ ЗАКОЛЕБАЛА ЭТА ВЕНДА
	_wfopen_s(&mFile, path.c_str(), append ? L"a+b" : L"wb");
#else
	mFile = fopen(path.toStdString().c_str(), append ? "a+b" : "wb");
#endif
	if (!mFile)
	{
		throw FileNotFoundException(path.toStdString().c_str());
	}
}

FileOutputStream::~FileOutputStream()
{
    close();
}

int FileOutputStream::write(const char* src, int size)
{
	return fwrite(src, 1, size, mFile);
}

void FileOutputStream::close() {
    if (mFile) {
        fclose(mFile);
        mFile = nullptr;
    }
}
