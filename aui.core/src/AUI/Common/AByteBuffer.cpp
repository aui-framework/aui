/*
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

#include <memory.h>
#include <cassert>
#include "AByteBuffer.h"

#include <glm/glm.hpp>


#include "AUI/IO/IInputStream.h"
#include "AUI/IO/AIOException.h"

AByteBuffer::AByteBuffer() {
    reserve(64);
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

AByteBuffer::AByteBuffer(const AByteBuffer& other) noexcept:
    mSize(other.mSize)
{
    reserve(other.mCapacity);
    memcpy(mBuffer, other.mBuffer, other.mSize);
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

