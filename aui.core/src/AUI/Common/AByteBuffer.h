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

#pragma once

#include "SharedPtr.h"

#include <cstddef>
#include <string>
#include <stdexcept>
#include <cassert>
#include "AUI/Core.h"
#include <AUI/Traits/serializable.h>
#include "AByteBufferView.h"


/**
 * @brief std::vector-like growing array for byte storage.
 * @ingroup core
 */
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
    AByteBuffer(AByteBufferView other) {
        reserve(other.size());
        memcpy(mBuffer, other.data(), other.size());
        mSize = other.size();
    }

    AByteBuffer(const AByteBuffer& other): AByteBuffer(AByteBufferView(other)) {}
    AByteBuffer(AByteBuffer&& other) noexcept;

    ~AByteBuffer();

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
     * @return Internal buffer.
     */
    char* data() const
    {
        return mBuffer;
    }

    /**
     * @brief Gets value of specified type by byte index relative to the beginning of internal buffer.
     * @tparam T data type
     * @param byteIndex byte offset realtive to the beginning of internal buffer
     * @return data
     */
    template <typename T>
    T& at(size_t byteIndex)
    {
        return *reinterpret_cast<T*>(mBuffer + byteIndex);
    }


    /**
     * @brief Gets value of specified type by byte index relative to the beginning of internal buffer.
     * @tparam T data type
     * @param byteIndex byte offset realtive to the beginning of internal buffer
     * @return data
     */
    template <typename T>
    const T& at(size_t byteIndex) const
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
     * @return size of payload (valid data)
     */
    size_t getSize() const noexcept {
        return mSize;
    }

    /**
     * @return true if size == 0
     */
    bool empty() const noexcept {
        return mSize == 0;
    }

    /**
     * @return size of payload (valid data)
     */
    size_t size() const noexcept {
        return mSize;
    }

    /**
     * @return size of whole buffer (including possibly invalid data)
     */
    size_t capacity() const noexcept {
        return mCapacity;
    }

    /**
     * @return size of internal buffer. Must be greater that getSize()
     */
    size_t getReserved() const noexcept {
        return mCapacity;
    }

    AByteBuffer& operator=(AByteBuffer&& other) noexcept {
        if (&other == this) {
            return *this;
        }

        mBuffer = other.mBuffer;
        mCapacity = other.mCapacity;
        mSize = other.mSize;

        other.mBuffer = nullptr;
        other.mCapacity = 0;
        other.mSize = 0;

        return *this;
    }
    AByteBuffer& operator=(const AByteBuffer& other) {
        if (&other == this) {
            return *this;
        }

        resize(other.size());
        std::memcpy(mBuffer, other.mBuffer, other.size());

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

    [[nodiscard]]
    AString toBase64String() const {
        return AByteBufferView(*this).toBase64String();
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