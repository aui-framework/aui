//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include <AUI/Platform/ErrorToException.h>
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
        aui::impl::unix::lastErrorToException("unable to open {}"_format(path));
    }
}

AFileInputStream::~AFileInputStream()
{
	fclose(mFile);
}

size_t AFileInputStream::read(char* dst, size_t size)
{
	size_t r = ::fread(dst, 1, size, mFile);
	return r;
}

void AFileInputStream::seek(std::streamoff offset, AFileInputStream::Seek dir) noexcept {
    fseek(mFile, offset, [&] {
        switch (dir) {
            case Seek::BEGIN:
                return SEEK_SET;
            case Seek::CURRENT:
                return SEEK_CUR;
            case Seek::END:
                return SEEK_END;
        }
        return 0;
    }());
}

void AFileInputStream::seek(std::streampos pos) noexcept {
    fseek(mFile, pos, SEEK_SET);
}

std::streampos AFileInputStream::tell() noexcept {
    return ftell(mFile);
}
