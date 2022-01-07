/**
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "AFileInputStream.h"


#include "AUI/Common/AString.h"

AFileInputStream::AFileInputStream(const AString& path)
{
#if AUI_PLATFORM_WIN
    // КАК ЖЕ ЗАКОЛЕБАЛА ЭТА ВЕНДА
    _wfopen_s(&mFile, path.c_str(), L"rb");
#else
    mFile = fopen(path.toStdString().c_str(), "rb");
#endif
	if (!mFile)
	{
		throw IOException("could not read from " + path.toStdString());
	}
}

AFileInputStream::~AFileInputStream()
{
	fclose(mFile);
}

int AFileInputStream::read(char* dst, int size)
{
	size_t r = ::fread(dst, 1, size, mFile);
	return r;
}

void AFileInputStream::seek(std::streamoff offset, AFileInputStream::Seek dir) {
    fseek(mFile, offset, [&] {
        switch (dir) {
            case Seek::BEGIN:
                return SEEK_SET;
            case Seek::CURRENT:
                return SEEK_CUR;
            case Seek::END:
                return SEEK_CUR;
        }
        return 0;
    }());
}

void AFileInputStream::seek(std::streampos pos) {
    fseek(mFile, pos, SEEK_SET);
}


