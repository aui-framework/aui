#pragma once

#include "ISeekableInputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Url/AUrl.h"

class AStrongByteBufferInputStream: public ISeekableInputStream {
private:
    AByteBuffer mRef;
    size_t mReadPos = 0;
    AStrongByteBufferInputStream() = default;

public:
    static std::shared_ptr<ISeekableInputStream> fromUrl(const AUrl& url) noexcept;

    explicit AStrongByteBufferInputStream(AByteBuffer buffer);
    ~AStrongByteBufferInputStream() override = default;

    bool isEof() override;
    void seek(std::streamoff offset, std::ios::seekdir seekDir) override;
    size_t tell() override;
    size_t read(char* dst, size_t size) override;
};