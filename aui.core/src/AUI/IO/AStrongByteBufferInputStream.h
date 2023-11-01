#pragma once

#include "ISeekableInputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Url/AUrl.h"

/**
 * @brief Input stream based on AByteBuffer with memory control, clears byte buffer on destruction
 * @ingroup io
 */
class API_AUI_CORE AStrongByteBufferInputStream: public ISeekableInputStream {
private:
    AByteBuffer mRef;
    size_t mReadPos = 0;

public:
    static _<AStrongByteBufferInputStream> fromUrl(const AUrl& url);

    explicit AStrongByteBufferInputStream(AByteBuffer buffer) noexcept: mRef(std::move(buffer)) {

    }
    ~AStrongByteBufferInputStream() override = default;

    bool isEof() override;

    void seek(std::streamoff offset, std::ios::seekdir seekDir) override;

    size_t tell() override;

    size_t read(char* dst, size_t size) override;
};