#include <memory.h>
#include <cassert>
#include "AByteBuffer.h"

#include <glm/glm.hpp>


#include "AUI/IO/IInputStream.h"
#include "AUI/IO/IOException.h"

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

AByteBuffer::AByteBuffer(const AByteBuffer& other) {
	reserve(other.mReserved);
	memcpy(mBuffer, other.mBuffer, other.mSize);
	mSize = other.mSize;
	mIndex = other.mIndex;
}

AByteBuffer::AByteBuffer(AByteBuffer&& other) noexcept
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
			reserve(mSize + size);
		}
		memcpy(mBuffer + mSize, buffer, size);
		mSize += size;
	}
}

void AByteBuffer::get(char* buffer, size_t size) const {
	if (mIndex + size > mSize)
		throw IOException("bytebuffer overflow");
	memcpy(buffer, mBuffer + mIndex, size);
	mIndex += size;
}

size_t AByteBuffer::getSize() const {
	return mSize;
}

size_t AByteBuffer::getReserved() const
{
	return mReserved;
}

const char* AByteBuffer::getCurrentPosAddress() const
{
	return mBuffer + mIndex;
}

char* AByteBuffer::getCurrentPosAddress()
{
	return mBuffer + mIndex;
}

/**
 * \return Количество доступных байт
 */
size_t AByteBuffer::getAvailable() const
{
	return mSize - mIndex;
}

void AByteBuffer::setCurrentPos(size_t p)
{
	mIndex = p;
}

char* AByteBuffer::getBuffer() const {
	return mBuffer;
}

void AByteBuffer::setSize(size_t s) {
	assert(s <= mReserved);
	mSize = s;
}

size_t AByteBuffer::getCurrentPos() const
{
	return mIndex;
}

bool AByteBuffer::operator==(const AByteBuffer& r) const {
	if (getSize() == r.getSize()) {
		return memcmp(getBuffer(), r.getBuffer(), getSize()) == 0;
	}
	return false;
}

bool AByteBuffer::operator!=(const AByteBuffer& r) const {
	return !(*this == r);
}

_<AByteBuffer> AByteBuffer::fromStream(_<IInputStream> is)
{
	auto buf = _new<AByteBuffer>();
	char tmp[4096];
	int last;
	while ((last = is->read(tmp, sizeof(tmp))) > 0)
	{
		buf->put(tmp, last);
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

_<AByteBuffer> AByteBuffer::fromString(const AString& string) {
    auto b = _new<AByteBuffer>();
    auto s = string.toStdString();
    b->put(s.data(), s.length());
    b->setCurrentPos(0);
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

