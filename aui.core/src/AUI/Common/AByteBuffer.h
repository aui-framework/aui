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

#pragma once

#include "SharedPtr.h"

#include <cstddef>
#include <string>
#include <stdexcept>
#include <cassert>
#include "AUI/Core.h"
#include <AUI/IO/IInputStream.h>
#include <AUI/IO/IOutputStream.h>
#include "AByteBufferRef.h"


class API_AUI_CORE AByteBuffer: public IInputStream, public IOutputStream {
private:
    char* mBuffer = nullptr;
    size_t mReserved = 0;
    size_t mSize = 0;
    mutable size_t mReadPos = 0;

public:
    AByteBuffer();
    AByteBuffer(const char* buffer, size_t size);
    explicit AByteBuffer(size_t reserved);
    AByteBuffer(const unsigned char* buffer, size_t size);
    ~AByteBuffer();

    AByteBuffer(const AByteBuffer& other) noexcept;
    AByteBuffer(AByteBuffer&& other) noexcept;

    AByteBufferRef ref() const {
        return { mBuffer, mSize };
    }

    size_t read(char* dst, size_t size) override;

    void write(const char* src, size_t size) override;

    /**
     * Reads exact <code>size</code> bytes from <code>stream</code>.
     * @param stream
     * @param size
     */
    void write(const IInputStream& stream, size_t size);

    operator AByteBufferRef() const {
        return ref();
    }

    void clear() {
        delete[] mBuffer;
        mBuffer = nullptr;
        mSize = mReserved = mReadPos = 0;
    }

    const char* readIterator() const {
        return mBuffer + mReadPos;
    }

    size_t getCurrentPos() const {
        return mReadPos;
    }

    void increaseCurrentPos(size_t by) const {
        mReadPos += by;
    }

    size_t availableToRead() const {
        return end() - readIterator();
    }

    /**
     * \brief Resizes internal buffer.
     */
    void reserve(size_t size);

    /**
     * \brief Increases internal buffer.
     */
    void increaseInternalBuffer(size_t size) {
        reserve(mReserved + size);
    }

    /**
     * \brief If <code>getReserved() - getSize()</code> is less than <code>size</code> increases internal buffer size
     *        enough to store <code>size</code> bytes.
     */
    void ensureReserved(size_t size) {
        auto availableToWrite = getAvailableToWrite();
        if (availableToWrite < size) {
            increaseInternalBuffer((glm::max)(getReserved() * 2, size_t(size - availableToWrite)));
        }
    }

    /**
     * @return delta between internal buffer size and payload size.
     */
    size_t getAvailableToWrite() const {
        return getReserved() - getSize();
    }


    /**
     * \return Internal buffer.
     */
    char* data() const
    {
        return mBuffer;
    }

    /**
     * \brief Gets value of specified type by byte index relative to the beginning of internal buffer.
     * \tparam T data type
     * \param byteIndex byte offset realtive to the beginning of internal buffer
     * \return data
     */
    template <typename T>
    T& at(size_t byteIndex)
    {
        return *reinterpret_cast<T*>(mBuffer + byteIndex);
    }

    /**
     * Forces new size of the buffer.
     * <dl>
     *  <dt><b>Sneaky assert:</b></dt>
     *  <dd>
     *      Assert fails when new size is greater that reserved buffer size. Use <code>AByteBuffer::resize</code> to
     *      avoid this.
     *  </dd>
     * </dl>
     * @param s new size of the payload
     */
    void setSize(size_t s) {
        assert(("size cannot be greater than reserved buffer size; did you mean AByteBuffer::resize?" && s <= mReserved));
        mSize = s;
    }
    /**
     * Forces new size of the buffer.
     * <dl>
     *  <dt><b>Sneaky assert:</b></dt>
     *  <dd>
     *      Assert fails when new size is greater that reserved buffer size. Use <code>AByteBuffer::resize</code> to
     *      avoid this.
     *  </dd>
     * </dl>
     * @param s new size of the payload
     */
    void increaseSize(size_t s) {
        mSize += s;
        assert(("size cannot be greater than reserved buffer size; did you mean AByteBuffer::resize?" && mSize <= mReserved));
    }

    /**
     * Resizes the buffer keeping it's contents. When reserved buffer size is less than the new size, buffer is
     * reallocated with new size.
     * @param s new size of the payload
     */
    void resize(size_t s) {
        if (mReserved < s) {
            reserve(s);
        }
        mSize = s;
    }

    /**
     * Resizes the buffer WITHOUT keeping it's contents. When reserved buffer size is differs from the new size, buffer is
     * reallocated with new size.
     * @param s new size of the payload
     */
    void reallocate(size_t s) {
        if (mReserved != s) {
            delete[] mBuffer;
            mBuffer = new char[s];
            mReserved = s;
        }
        mSize = s;
    }

    /**
     * \return size of payload (valid data)
     */
    size_t getSize() const {
        return mSize;
    }

    /**
     * \return size of payload (valid data)
     */
    size_t size() const {
        return mSize;
    }

    /**
     * \return size of internal buffer. Must be greater that getSize()
     */
    size_t getReserved() const {
        return mReserved;
    }


    /**
     * \param p new reading offset
     */
    void setCurrentPos(size_t p) const {
        mReadPos = p;
    }


    AByteBuffer& operator=(AByteBuffer&& other) {
        mBuffer = other.mBuffer;
        mReadPos = other.mReadPos;
        mReserved = other.mReserved;
        mSize = other.mSize;

        other.mBuffer = nullptr;
        other.mReadPos = 0;
        other.mReserved = 0;
        other.mSize = 0;
        return *this;
    }

    bool operator==(const AByteBuffer& r) const;
    bool operator!=(const AByteBuffer& r) const;

    char* begin()
    {
        return mBuffer;
    }
    char* end()
    {
        return mBuffer + mSize;
    }
    char* endReserved()
    {
        return mBuffer + mReserved;
    }
    const char* begin() const
    {
        return mBuffer;
    }
    const char* end() const
    {
        return mBuffer + mSize;
    }

    AString toHexString();

    static AByteBuffer fromStream(IInputStream& is);
    static AByteBuffer fromStream(IInputStream& is, size_t sizeRestriction);
    static AByteBuffer fromStream(IInputStream&& is) {
		return fromStream(is);
	}
    static AByteBuffer fromStream(IInputStream&& is, size_t sizeRestriction) {
		return fromStream(is, sizeRestriction);
	}
    static AByteBuffer fromStream(const _<IInputStream>& is) {
        return fromStream(*is);
    }
    static AByteBuffer fromStream(const _<IInputStream>& is, size_t sizeRestriction) {
        return fromStream(*is, sizeRestriction);
    }
    static AByteBuffer fromString(const AString& string);
    static AByteBuffer fromHexString(const AString& string);
    static AByteBuffer fromBase64String(const AString& encodedString);
};

API_AUI_CORE std::ostream& operator<<(std::ostream& o, const AByteBuffer& r);
