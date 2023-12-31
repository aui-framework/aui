#include "AStrongByteBufferInputStream.h"


_<AStrongByteBufferInputStream> AStrongByteBufferInputStream::fromUrl(const AUrl& url) {
    return _new<AStrongByteBufferInputStream>(AByteBuffer::fromStream(AUrl(url).open()));
}

bool AStrongByteBufferInputStream::isEof() {
    return mReadPos == mRef.size();
}

void AStrongByteBufferInputStream::seek(std::streamoff offset, std::ios::seekdir seekDir) {
    switch (seekDir) {
        case std::ios::beg: mReadPos = offset; break;
        case std::ios::cur: mReadPos += offset; break;
        case std::ios::end: mReadPos = mRef.size() + offset; break;
    }
}

size_t AStrongByteBufferInputStream::tell() {
    return mReadPos;
}

size_t AStrongByteBufferInputStream::read(char* dst, size_t size) {
    size_t toRead = glm::min(mReadPos + size, mRef.size()) - mReadPos;
    std::memcpy(dst, mRef.data() + mReadPos, toRead);
    mReadPos += toRead;
    return toRead;
}
