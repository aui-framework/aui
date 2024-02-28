// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include <memory.h>
#include <cassert>
#include "AByteBuffer.h"

#include <glm/glm.hpp>


#include "AUI/IO/IInputStream.h"
#include "AUI/IO/AIOException.h"

AByteBuffer::AByteBuffer() {
}
AByteBuffer::AByteBuffer(size_t initialCapacity) {
    reserve(initialCapacity);
}

AByteBuffer::AByteBuffer(const char* buffer, size_t size)
{
    write(buffer, size);
}

AByteBuffer::AByteBuffer(const unsigned char* buffer, size_t size)
{
    write(reinterpret_cast<const char*>(buffer), size);
}


AByteBuffer::AByteBuffer(AByteBuffer&& other) noexcept
{
    AByteBuffer::operator=(std::move(other));
}

void AByteBuffer::reserve(size_t size) {
    char* buffer = new char[size];
    if (mBuffer) {
        memcpy(buffer, mBuffer, glm::min(mCapacity, size));
        delete[] mBuffer;
    }
    mCapacity = size;
    mBuffer = buffer;
}

void AByteBuffer::write(const char* buffer, size_t size) {
    if (size) {
        if (mSize + size > mCapacity) {
            reserve(mSize * 2 + size);
        }
        memcpy(end(), buffer, size);
        mSize += size;
    }
}

bool AByteBuffer::operator==(const AByteBuffer& r) const {
    if (getSize() == r.getSize()) {
        return memcmp(data(), r.data(), getSize()) == 0;
    }
    return false;
}

bool AByteBuffer::operator!=(const AByteBuffer& r) const {
    return !(*this == r);
}

AByteBuffer AByteBuffer::fromStream(aui::no_escape<IInputStream> is)
{
    AByteBuffer buf;
    char tmp[0x10000];

    for (size_t last; (last = is->read(tmp, sizeof(tmp))) > 0;)
    {
        buf.write(tmp, last);
    }
    return buf;
}

AByteBuffer AByteBuffer::fromStream(aui::no_escape<IInputStream> is, size_t sizeRestriction) {
    AByteBuffer buf;
    buf.reserve(sizeRestriction);
    char tmp[4096];
    for (size_t last; (last = is->read(tmp, glm::min(sizeof(tmp), sizeRestriction))) > 0; sizeRestriction -= last)
    {
        buf.write(tmp, last);
    }
    return buf;
}


std::ostream& operator<<(std::ostream& o, AByteBufferView buffer)
{
    char formatBuf[8];
    o << '[';
    for (auto c : buffer)
    {
        sprintf(formatBuf, "%02x ", std::uint8_t(c));
        o << formatBuf;
    }
    o << ']';
    return o;
}

AByteBuffer::~AByteBuffer() {
    delete[] mBuffer;
    mBuffer = nullptr;
}

AByteBuffer AByteBuffer::fromString(const AString& string) {
    AByteBuffer b;
    auto s = string.toStdString();
    b.write(s.data(), s.length());
    return b;
}


uint8_t hexCharToNumber(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return 10 + c - 'a';
    if (c >= 'A' && c <= 'F')
        return 10 + c - 'A';
    return -1;
}

AByteBuffer AByteBuffer::fromHexString(const AString& string) {
    AByteBuffer result;
    result.reserve(string.length() / 2);

    for (int i = 0; i < string.length(); i += 2) {
        uint8_t byte = (hexCharToNumber(char(string[i])) << 4u) | hexCharToNumber(char(string[i + 1]));
        result << byte;
    }

    return result;
}


void AByteBuffer::write(IInputStream& stream, size_t size) {
    auto avail = mCapacity - mSize;
    if (avail < size) {
        reserve(mCapacity + size);
    }
    stream.readExact(end(), size);
    mSize += size;
}

