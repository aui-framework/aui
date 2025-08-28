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

#include <memory.h>
#include <cassert>
#include "AByteBuffer.h"

#include <glm/glm.hpp>
#include <AUI/Util/kAUI.h>


#include "AUI/Common/AException.h"
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

void AByteBuffer::write(const char* src, size_t size) {
    if (size) {
        if (mSize + size > mCapacity) {
            reserve(mSize * 2 + size);
        }
        memcpy(end(), src, size);
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
    buf.reserve(0x1000);

    for (size_t last; (last = is->read(buf.end(), buf.getAvailableToWrite())) > 0;)
    {
        buf.mSize += last;

        if (buf.getAvailableToWrite() < 0x100) {
            buf.grow(buf.capacity());
        }
    }
    return buf;
}

AByteBuffer AByteBuffer::fromStream(aui::no_escape<IInputStream> is, size_t sizeRestriction) {
    AByteBuffer buf;
    buf.reserve(0x1000);

    for (size_t last; (last = is->read(buf.end(), buf.getAvailableToWrite())) > 0;)
    {
        buf.mSize += last;

        if (buf.getAvailableToWrite() < 0x100) {
            auto newCapacity = glm::min(buf.capacity() * 2, sizeRestriction);
            if (buf.capacity() == newCapacity && buf.getAvailableToWrite()) {
                break;
            }
            buf.reserve(newCapacity);
        }
    }
    return buf;
}


std::ostream& operator<<(std::ostream& o, AByteBufferView buffer)
{
    char formatBuf[8];
    o << '[';
    for (auto c : buffer)
    {
        std::snprintf(formatBuf, sizeof(formatBuf), "%02x ",
            static_cast<unsigned>(static_cast<uint8_t>(c))
        );
        o << formatBuf;
    }
    o << ']';
    return o;
}

AByteBuffer::~AByteBuffer() {
    delete[] mBuffer;
    mBuffer = nullptr;
}

AByteBuffer AByteBuffer::fromString(AStringView string) {
    AByteBuffer b;
    b.write(string.data(), string.length());
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

AByteBuffer AByteBuffer::fromHexString(AStringView string) {
    if (string.length() % 2 != 0) {
        throw AException("invalid string length");
    }
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
    char* begin = end();
    char* end = this->end() + size;
    AUI_DEFER {
        mSize = begin - this->begin();
    };
    while (begin != end) {
        size_t r = stream.read(begin, end - begin);
        if (r == 0) {
            throw AEOFException();
        }
        begin += r;
    }
}

