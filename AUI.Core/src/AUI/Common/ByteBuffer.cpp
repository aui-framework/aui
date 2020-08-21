#include <memory.h>
#include <cassert>
#include "ByteBuffer.h"

#include <glm/glm.hpp>


#include "AUI/IO/IInputStream.h"
#include "AUI/IO/IOException.h"

ByteBuffer::ByteBuffer() {
	reserve(64);
}

ByteBuffer::ByteBuffer(const char* buffer, size_t size)
{
	put(buffer, size);
}

ByteBuffer::ByteBuffer(const unsigned char* buffer, size_t size)
{
	put(reinterpret_cast<const char*>(buffer), size);
}

ByteBuffer::ByteBuffer(const ByteBuffer& other) {
	reserve(other.mReserved);
	memcpy(mBuffer, other.mBuffer, other.mSize);
	mSize = other.mSize;
	mIndex = other.mIndex;
}

ByteBuffer::ByteBuffer(ByteBuffer&& other) noexcept
{
	mBuffer = other.mBuffer;
	mIndex = other.mIndex;
	mReserved = other.mReserved;
	mSize = other.mSize;

	other.mBuffer = 0;
	other.mIndex = 0;
	other.mReserved = 0;
	other.mSize = 0;
	other.reserve(64);
}

void ByteBuffer::reserve(size_t size) {
	char* buffer = new char[size];
	if (mBuffer) {
		memcpy(buffer, mBuffer, glm::min(mReserved, size));
		delete[] mBuffer;
	}
	mReserved = size;
	mBuffer = buffer;
}

void ByteBuffer::put(const char* buffer, size_t size) {
	if (size) {
		if (mSize + size > mReserved) {
			reserve(mSize + size);
		}
		memcpy(mBuffer + mSize, buffer, size);
		mSize += size;
	}
}

void ByteBuffer::get(char* buffer, size_t size) const {
	if (mIndex + size > mSize)
		throw IOException("bytebuffer overflow");
	memcpy(buffer, mBuffer + mIndex, size);
	mIndex += size;
}

size_t ByteBuffer::getSize() const {
	return mSize;
}

size_t ByteBuffer::getReserved() const
{
	return mReserved;
}

const char* ByteBuffer::getCurrentPosAddress() const
{
	return mBuffer + mIndex;
}

char* ByteBuffer::getCurrentPosAddress()
{
	return mBuffer + mIndex;
}

/**
 * \return Количество доступных байт
 */
size_t ByteBuffer::getAvailable() const
{
	return mSize - mIndex;
}

void ByteBuffer::setCurrentPos(size_t p)
{
	mIndex = p;
}

char* ByteBuffer::getBuffer() const {
	return mBuffer;
}

void ByteBuffer::setSize(size_t s) {
	assert(s <= mReserved);
	mSize = s;
}

size_t ByteBuffer::getCurrentPos() const
{
	return mIndex;
}

bool ByteBuffer::operator==(const ByteBuffer& r) const {
	if (getSize() == r.getSize()) {
		return memcmp(getBuffer(), r.getBuffer(), getSize()) == 0;
	}
	return false;
}

bool ByteBuffer::operator!=(const ByteBuffer& r) const {
	return !(*this == r);
}

_<ByteBuffer> ByteBuffer::fromStream(_<IInputStream> is)
{
	auto buf = _new<ByteBuffer>();
	char tmp[4096];
	int last;
	while ((last = is->read(tmp, sizeof(tmp))) > 0)
	{
		buf->put(tmp, last);
	}
	return buf;
}

std::ostream& operator<<(std::ostream& o, const ByteBuffer& r)
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

ByteBuffer::~ByteBuffer() {
	delete[] mBuffer;
	mBuffer = nullptr;
}

