#include "AStrongByteBufferInputStream.h"


_<AStrongByteBufferInputStream> AStrongByteBufferInputStream::fromUrl(const AUrl& url) {
    return _new<AStrongByteBufferInputStream>(AByteBuffer::fromStream(AUrl(url).open()));
}

bool AStrongByteBufferInputStream::isEof() {
    return mReadPos == buffer().size();
}

void AStrongByteBufferInputStream::seek(std::streamoff offset, Seek seekDir) {
    switch (seekDir) {
        case Seek::BEGIN:
            mReadPos = offset;
            break;
        case Seek::CURRENT:
            mReadPos += offset;
            break;
        case Seek::END:
            mReadPos = buffer().size() + offset;
            break;
        default:
            break;
    }
}

std::streampos AStrongByteBufferInputStream::tell() noexcept {
    return mReadPos;
}

size_t AStrongByteBufferInputStream::read(char* dst, size_t size) {
    size_t toRead = glm::min(mReadPos + size, buffer().size()) - mReadPos;
    std::memcpy(dst, buffer().data() + mReadPos, toRead);
    mReadPos += toRead;
    return toRead;
}
