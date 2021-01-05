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
 
 * Original code located at https://github.com/Alex2772/aui
 * =====================================================================================================================
 */

#pragma once

#include "SharedPtr.h"

#include <cstddef>
#include <string>
#include <stdexcept>
#include "AUI/Core.h"

class IInputStream;

class API_AUI_CORE AByteBuffer {
private:
	char* mBuffer = nullptr;
	size_t mReserved = 0;
	size_t mSize = 0;
	mutable size_t mIndex = 0;

public:
	AByteBuffer();
	AByteBuffer(const char* buffer, size_t size);
	AByteBuffer(const unsigned char* buffer, size_t size);
	~AByteBuffer();
	
	AByteBuffer(const AByteBuffer& other) noexcept;
	AByteBuffer(AByteBuffer&& other) noexcept;
	void reserve(size_t size);
	void put(const char* buffer, size_t size);
	void get(char* buffer, size_t size) const;

	char* data() const
	{
		return mBuffer;
	}

	template <typename T>
	T& at(size_t byteIndex)
	{
		return *reinterpret_cast<T*>(mBuffer + byteIndex);
	}

	template <typename T>
	AByteBuffer& operator<<(const T& data) {
		put(reinterpret_cast<const char*>(&data), sizeof(T));
		return *this;
	}
	template <typename T>
	const AByteBuffer& operator>>(T& dst) const {
		get(reinterpret_cast<char*>(&dst), sizeof(T));
		return *this;
	}

	AByteBuffer& operator<<(const std::string& data) {
		*this << uint32_t(data.length());
		put(data.c_str(), data.length());
		return *this;
	}
	const AByteBuffer& operator>>(std::string& dst) const {
		uint32_t s;
		*this >> s;
		if (s > 128)
			throw std::runtime_error(std::string("illegal string size: ") + std::to_string(s));
		dst.resize(s);
		get(&(dst[0]), s);
		return *this;
	}

	AByteBuffer& operator<<(const AByteBuffer& data) {
		put(data.getBuffer(), data.getSize());
		return *this;
	}
	const AByteBuffer& operator>>(AByteBuffer& dst) const {
		dst.put(getBuffer(), getSize());
		return *this;
	}


	void setSize(size_t s);
	char* getBuffer() const;
	size_t getSize() const;
	size_t getReserved() const;
	const char* getCurrentPosAddress() const;
	char* getCurrentPosAddress();
	size_t getAvailable() const;

	void setCurrentPos(size_t p);
	size_t getCurrentPos() const;

	AByteBuffer& operator=(AByteBuffer&& other) {
        mBuffer = other.mBuffer;
        mIndex = other.mIndex;
        mReserved = other.mReserved;
        mSize = other.mSize;

        other.mBuffer = nullptr;
        other.mIndex = 0;
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

	AString toHexString();

	static AByteBuffer fromStream(const _<IInputStream>& is);
	static AByteBuffer fromStream(const _<IInputStream>& is, size_t sizeRestriction);
    static AByteBuffer fromString(const AString& string);
    static AByteBuffer fromHexString(const AString& string);
    static AByteBuffer fromBase64String(const AString& encodedString);
};

API_AUI_CORE std::ostream& operator<<(std::ostream& o, const AByteBuffer& r);