// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "AFileOutputStream.h"

#include "AUI/Common/AString.h"
#include "AUI/Platform/ErrorToException.h"

#if AUI_PLATFORM_WIN
#include <share.h>
#endif

AFileOutputStream::AFileOutputStream(AString path, bool append): mPath(std::move(path)), mFile(nullptr)
{
    open(append);
}

AFileOutputStream::~AFileOutputStream()
{
    close();
}

void AFileOutputStream::write(const char* src, size_t size)
{
    if (mFile == nullptr) {
        throw AIOException("Write attempt to not opened file: " + mPath);
    }

    if (size == 0) return;
	while (size) {
        auto v = fwrite(src, 1, size, mFile);
        if (v == 0) {
            aui::impl::unix_based::lastErrorToException(mPath);
        }
        size -= v;
        src += v;
    }
}

void AFileOutputStream::close() {
    if (mFile) {
        fclose(mFile);
        mFile = nullptr;
    }
}

void AFileOutputStream::open(bool append) {
    if (mFile != nullptr) {
        throw AIOException("Trying to open already opened file: " + mPath);
    }

#if AUI_PLATFORM_WIN
    mFile = _wfsopen(mPath.c_str(), append ? L"a+b" : L"wb", _SH_DENYWR);
#else
    mFile = fopen(mPath.toStdString().c_str(), append ? "a+b" : "wb");
#endif
    if (!mFile)
    {
        aui::impl::lastErrorToException("AFileOutputStream: could not open {}"_format(mPath));
    }
}
