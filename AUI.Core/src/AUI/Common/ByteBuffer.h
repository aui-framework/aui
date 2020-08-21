#pragma once

#include "SharedPtr.h"

#include <cstddef>
#include <string>
#include <stdexcept>
#include "AUI/Core.h"

class IInputStream;

class API_AUI_CORE ByteBuffer {
private:
	char* mBuffer = nullptr;
	size_t mReserved = 0;
	size_t mSize = 0;
	mutable size_t mIndex = 0;

public:
	ByteBuffer();
	ByteBuffer(const char* buffer, size_t size);
	ByteBuffer(const unsigned char* buffer, size_t size);
	~ByteBuffer();
	
	ByteBuffer(const ByteBuffer& other);
	ByteBuffer(ByteBuffer&& other) noexcept;
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
	ByteBuffer& operator<<(const T& data) {
		put(reinterpret_cast<const char*>(&data), sizeof(T));
		return *this;
	}
	template <typename T>
	const ByteBuffer& operator>>(T& dst) const {
		get(reinterpret_cast<char*>(&dst), sizeof(T));
		return *this;
	}

	ByteBuffer& operator<<(const std::string& data) {
		*this << uint32_t(data.length());
		put(data.c_str(), data.length());
		return *this;
	}
	const ByteBuffer& operator>>(std::string& dst) const {
		uint32_t s;
		*this >> s;
		if (s > 128)
			throw std::runtime_error(std::string("illegal string size: ") + std::to_string(s));
		dst.resize(s);
		get(&(dst[0]), s);
		return *this;
	}

	ByteBuffer& operator<<(const ByteBuffer& data) {
		put(data.getBuffer(), data.getSize());
		return *this;
	}
	const ByteBuffer& operator>>(ByteBuffer& dst) const {
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

	bool operator==(const ByteBuffer& r) const;
	bool operator!=(const ByteBuffer& r) const;

	char* begin()
	{
		return mBuffer;
	}
	char* end()
	{
		return mBuffer + mSize;
	}


	static _<ByteBuffer> fromStream(_<IInputStream> is);
};

API_AUI_CORE std::ostream& operator<<(std::ostream& o, const ByteBuffer& r);