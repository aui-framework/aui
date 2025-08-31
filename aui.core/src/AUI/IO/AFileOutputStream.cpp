/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "AFileOutputStream.h"

#include "AUI/Common/AString.h"
#include "AUI/Common/AByteBuffer.h"
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
    AByteBuffer pathU16 = mPath.encode(AStringEncoding::UTF16);
    mFile = _wfsopen(reinterpret_cast<const wchar_t*>(pathU16.data()), append ? L"a+b" : L"wb", _SH_DENYWR);
#else
    mFile = fopen(mPath.toStdString().c_str(), append ? "a+b" : "wb");
#endif
    if (!mFile)
    {
        aui::impl::lastErrorToException("AFileOutputStream: could not open {}"_format(mPath));
    }
}
