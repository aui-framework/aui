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

#pragma once

#include "SharedPtr.h"

#include <cstddef>
#include <string>
#include <stdexcept>
#include <cassert>
#include "AUI/Core.h"
#include <AUI/Traits/serializable.h>
#include "AByteBufferView.h"


class API_AUI_CORE AByteBuffer final: public IOutputStream {
private:
    char* mBuffer = nullptr;
    size_t mCapacity = 0;
    size_t mSize = 0;

public:
    AByteBuffer();
    AByteBuffer(const char* buffer, size_t size);
    explicit AByteBuffer(size_t initialCapacity);
    AByteBuffer(const unsigned char* buffer, size_t size);
    ~AByteBuffer();

    AByteBuffer(const AByteBuffer& other) noexcept;
    AByteBuffer(AByteBuffer&& other) noexcept;

    void write(const char* src, size_t size) override;


    [[nodiscard]]
    AByteBufferView slice(std::size_t offset, std::size_t size) const noexcept {
        return operator AByteBufferView().slice(offset, size);
    }

    /**
     * Reads exact <code>size</code> bytes from <code>stream</code>.
     * @param stream
     * @param size
     */
    void write(IInputStream& stream, size_t size);

    operator AByteBufferView() const noexcept {
        return { mBuffer, mSize };
    }

    void clear() {
        delete[] mBuffer;
        mBuffer = nullptr;
        mSize = mCapacity = 0;
    }

    /**
     * @brief Resizes internal buffer.
     */
    void reserve(size_t size);

    /**
     * @brief Increases internal buffer.
     */
    void increaseInternalBuffer(size_t size) {
        reserve(mCapacity + size);
    }

    /**
     * @brief If <code>getReserved() - getSize()</code> is less than <code>size</code> increases internal buffer size
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
     * @brief Gets value of specified type by byte index relative to the beginning of internal buffer.
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
        assert(("size cannot be greater than reserved buffer size; did you mean AByteBuffer::resize?" && s <= mCapacity));
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
        assert(("size cannot be greater than reserved buffer size; did you mean AByteBuffer::resize?" && mSize <= mCapacity));
    }

    /**
     * Resizes the buffer keeping it's contents. When reserved buffer size is less than the new size, buffer is
     * reallocated with new size.
     * @param s new size of the payload
     */
    void resize(size_t s) {
        if (mCapacity < s) {
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
        if (mCapacity != s) {
            delete[] mBuffer;
            mBuffer = new char[s];
            mCapacity = s;
        }
        mSize = s;
    }

    /**
     * \return size of payload (valid data)
     */
    size_t getSize() const noexcept {
        return mSize;
    }

    /**
     * \return size of payload (valid data)
     */
    size_t size() const noexcept {
        return mSize;
    }

    /**
     * \return size of whole buffer (including possibly invalid data)
     */
    size_t capacity() const noexcept {
        return mCapacity;
    }

    /**
     * \return size of internal buffer. Must be greater that getSize()
     */
    size_t getReserved() const noexcept {
        return mCapacity;
    }

    AByteBuffer& operator=(AByteBuffer&& other) {
        mBuffer = other.mBuffer;
        mCapacity = other.mCapacity;
        mSize = other.mSize;

        other.mBuffer = nullptr;
        other.mCapacity = 0;
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
        return mBuffer + mCapacity;
    }
    const char* begin() const
    {
        return mBuffer;
    }
    const char* end() const
    {
        return mBuffer + mSize;
    }


    template<typename T>
    T as() const {
        return AByteBufferView(*this).template as<T>();
    }

    [[nodiscard]]
    AString toHexString() const {
        return AByteBufferView(*this).toHexString();
    }

    static AByteBuffer fromStream(aui::no_escape<IInputStream> is);
    static AByteBuffer fromStream(aui::no_escape<IInputStream> is, size_t sizeRestriction);

    static AByteBuffer fromString(const AString& string);
    static AByteBuffer fromHexString(const AString& string);
    static AByteBuffer fromBase64String(const AString& encodedString);
};

API_AUI_CORE std::ostream& operator<<(std::ostream& o, AByteBufferView buffer);

template<>
struct ASerializable<AByteBuffer> {
    static void write(IOutputStream& os, const AByteBuffer& value) {
        os.write(value.data(), value.size());
    }
};