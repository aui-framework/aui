/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
        aui::impl::unix_based::lastErrorToException("unable to open {}"_format(path));
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
