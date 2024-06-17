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

#include "AStringStream.h"


AStringStream::AStringStream(const AString& string): mString(string.toStdString())
{
}

AStringStream::AStringStream(std::string string) : mString(std::move(string)) {
}

AStringStream::AStringStream() = default;

size_t AStringStream::read(char* dst, size_t size)
{
	if (mReadPos >= mString.length())
		return 0;

	size_t toRead = glm::min(size, mString.length() - mReadPos);
	memcpy(dst, mString.c_str() + mReadPos, toRead);
    mReadPos += toRead;
	return toRead;
}

void AStringStream::write(const char *src, size_t size) {
    mString.append(src, src + size);
}

void AStringStream::seekRead(size_t position) {
	mReadPos = position;
}
