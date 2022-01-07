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

class IInputStream;

class API_AUI_CORE AByteBuffer {
private:
    char* mBuffer = nullptr;
    size_t mReserved = 0;
    size_t mSize = 0;
    mutable size_t mCurrentPos = 0;

public:
    AByteBuffer();
    AByteBuffer(const char* buffer, size_t size);
    AByteBuffer(const unsigned char* buffer, size_t size);
    ~AByteBuffer();

    AByteBuffer(const AByteBuffer& other) noexcept;
    AByteBuffer(AByteBuffer&& other) noexcept;

    void clear() {
        delete[] mBuffer;
        mBuffer = nullptr;
        mSize = mReserved = mCurrentPos = 0;
    }

    /**
     * \brief Resizes internal buffer.
     */
    void reserve(size_t size);

    /**
     * \brief Appends data to the buffer. Twices internal buffer's size if needed.
     * \param buffer pointer to data
     * \param size data size to write in bytes
     */
    void put(const char* buffer, size_t size);

    /**
     * \brief Retrieves next <code>size</code> bytes from the buffer.
     * \param buffer pointer to destination buffer
     * \param size data size to read in bytes
     */
    void get(char* buffer, size_t size) const;

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
     * \brief Wrapper around put
     */
    template <typename T>
    AByteBuffer& operator<<(const T& data) {
        // static_assert(std::is_standard_layout_v<T>, "data is too complex to be put onto buffer");
        put(reinterpret_cast<const char*>(&data), sizeof(T));
        return *this;
    }


    /**
     * \brief Wrapper around get
     */
    template <typename T>
    const AByteBuffer& operator>>(T& dst) const {
        // static_assert(std::is_standard_layout_v<T>, "data is too complex to be read from buffer");
        get(reinterpret_cast<char*>(&dst), sizeof(T));
        return *this;
    }

    /**
     * \brief Puts string size and its contents to the buffer.
     */
    AByteBuffer& operator<<(const std::string& data) {
        *this << uint32_t(data.length());
        put(data.c_str(), data.length());
        return *this;
    }

    /**
     * \brief Reads string size and string contents.
     */
    const AByteBuffer& operator>>(std::string& dst) const {
        uint32_t s;
        *this >> s;
        if (s > 128)
            throw std::runtime_error(std::string("illegal string size: ") + std::to_string(s));
        dst.resize(s);
        get(&(dst[0]), s);
        return *this;
    }

    /**
     * \brief Puts another byte buffer.
     */
    AByteBuffer& operator<<(const AByteBuffer& data) {
        put(data.data(), data.getSize());
        return *this;
    }

    /**
     * \brief Put this bytebuffer to another byte buffer.
     */
    const AByteBuffer& operator>>(AByteBuffer& dst) const {
        dst.put(data(), getSize());
        return *this;
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
     * \return size of internal buffer. Must be greater that getSize()
     */
    size_t getReserved() const {
        return mReserved;
    }

    /**
     * \return pointer to data including reading/writing offset.
     */
    const char* getCurrentPosAddress() const {
        return mBuffer + mCurrentPos;
    }

    /**
     * \return pointer to data including reading/writing offset.
     */
    char* getCurrentPosAddress() {
        return mBuffer + mCurrentPos;
    }

    /**
     * \return number of available (unread) bytes.
     */
    size_t getAvailable() const
    {
        return mSize - mCurrentPos;
    }

    /**
     * \param p new reading/writing offset
     */
    void setCurrentPos(size_t p) const {
        mCurrentPos = p;
    }

    /**
     * \return reading/writing offset
     */
    size_t getCurrentPos() const {
        return mCurrentPos;
    }

    AByteBuffer& operator=(AByteBuffer&& other) {
        mBuffer = other.mBuffer;
        mCurrentPos = other.mCurrentPos;
        mReserved = other.mReserved;
        mSize = other.mSize;

        other.mBuffer = nullptr;
        other.mCurrentPos = 0;
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