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

#include <memory.h>
#include <cassert>
#include "AByteBuffer.h"

#include <glm/glm.hpp>


#include "AUI/IO/IInputStream.h"
#include "AUI/IO/AIOException.h"

AByteBuffer::AByteBuffer() {
    reserve(64);
}

AByteBuffer::AByteBuffer(const char* buffer, size_t size)
{
    put(buffer, size);
}

AByteBuffer::AByteBuffer(const unsigned char* buffer, size_t size)
{
    put(reinterpret_cast<const char*>(buffer), size);
}

AByteBuffer::AByteBuffer(const AByteBuffer& other) noexcept {
    reserve(other.mReserved);
    memcpy(mBuffer, other.mBuffer, other.mSize);
    mSize = other.mSize;
    mCurrentPos = other.mCurrentPos;
}

AByteBuffer::AByteBuffer(AByteBuffer&& other) noexcept
{
    AByteBuffer::operator=(std::move(other));
}

void AByteBuffer::reserve(size_t size) {
    char* buffer = new char[size];
    if (mBuffer) {
        memcpy(buffer, mBuffer, glm::min(mReserved, size));
        delete[] mBuffer;
    }
    mReserved = size;
    mBuffer = buffer;
}

void AByteBuffer::put(const char* buffer, size_t size) {
    if (size) {
        if (mSize + size > mReserved) {
            reserve(mSize * 2 + size);
        }
        memcpy(mBuffer + mSize, buffer, size);
        mSize += size;
    }
}

void AByteBuffer::get(char* buffer, size_t size) const {
    if (mCurrentPos + size > mSize)
        throw AIOException("bytebuffer overflow");
    memcpy(buffer, mBuffer + mCurrentPos, size);
    mCurrentPos += size;
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

AByteBuffer AByteBuffer::fromStream(IInputStream& is)
{
    AByteBuffer buf;
    char tmp[0x10000];

    for (size_t last; (last = is.read(tmp, sizeof(tmp))) > 0;)
    {
        buf.put(tmp, last);
    }
    return buf;
}

AByteBuffer AByteBuffer::fromStream(IInputStream& is, size_t sizeRestriction) {
    AByteBuffer buf;
    char tmp[4096];
    for (size_t last; (last = is.read(tmp, sizeof(tmp))) > 0 && buf.getSize() < sizeRestriction;)
    {
        buf.put(tmp, last);
    }
    return buf;
}


std::ostream& operator<<(std::ostream& o, const AByteBuffer& r)
{
    char formatBuf[8];
    o << '[';
    while (r.getAvailable())
    {
        unsigned char c;
        r >> c;
        sprintf(formatBuf, "%02x ", c);
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
    b.put(s.data(), s.length());
    b.setCurrentPos(0);
    return b;
}

AString AByteBuffer::toHexString() {
    AString result;
    result.reserve(getSize() * 2 + 10);
    char buf[8];

    for (size_t i = 0; i < getSize(); ++i) {
        sprintf(buf, "%02x", static_cast<unsigned>(mBuffer[i]) & 0xff);
        result += buf;
    }
    return result;
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

static const std::string BASE64_CHARS =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}
AByteBuffer AByteBuffer::fromBase64String(const AString& encodedString) {
    int in_len = encodedString.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    AByteBuffer ret;

    while (in_len-- && ( encodedString[in_] != '=') && is_base64(encodedString[in_])) {
        char_array_4[i++] = encodedString[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = BASE64_CHARS.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret << char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = BASE64_CHARS.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret << char_array_3[j];
    }

    return ret;
}

