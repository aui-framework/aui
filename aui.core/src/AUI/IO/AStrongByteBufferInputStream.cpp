#include "AStrongByteBufferInputStream.h"


_<AStrongByteBufferInputStream> AStrongByteBufferInputStream::fromUrl(const AUrl& url) {
    return _new<AStrongByteBufferInputStream>(AByteBuffer::fromStream(AUrl(url).open()));
}

AStrongByteBufferInputStream::AStrongByteBufferInputStream(AByteBuffer&& buffer) : mBuffer(std::move(buffer)) {
}

bool AStrongByteBufferInputStream::isEof() {
    return mReadPos == mBuffer.size();
}

void AStrongByteBufferInputStream::seek(std::streamoff offset, std::ios::seekdir seekDir) {
    switch (seekDir) {
        case std::ios::beg: mReadPos = offset; break;
        case std::ios::cur: mReadPos += offset; break;
        case std::ios::end: mReadPos = mBuffer.size() + offset; break;
    }
}

size_t AStrongByteBufferInputStream::tell() {
    return mReadPos;
}

size_t AStrongByteBufferInputStream::read(char* dst, size_t size) {
    size_t toRead = glm::min(mReadPos + size, mBuffer.size()) - mReadPos;
    std::memcpy(dst, mBuffer.data() + mReadPos, toRead);
    mReadPos += toRead;
    return toRead;
}
