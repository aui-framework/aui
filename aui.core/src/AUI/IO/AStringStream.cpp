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

#include "AStringStream.h"


AStringStream::AStringStream(const AString& string): mString(string.toStdString())
{
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
