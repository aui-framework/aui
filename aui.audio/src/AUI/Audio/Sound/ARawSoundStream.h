#pragma once

#include "AUI/Audio/Sound/ISoundInputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"

/**
 * @brief Sound stream for storing raw sound data, useful for storing recorded sound data
 * @ingroup audio
 */
class ARawSoundStream : public ISoundInputStream {
public:
    ARawSoundStream(AAudioFormat format, AByteBuffer data) : mFormat(std::move(format)),
                                                             mStream(_new<AStrongByteBufferInputStream>(std::move(data))) { }

    size_t read(char* dst, size_t size) override {
        return mStream->read(dst, size);
    }

    AAudioFormat info() override {
        return mFormat;
    }

    void rewind() override {
        mStream->seek(0, std::ios::beg);
    }

private:
    AAudioFormat mFormat;
    _<AStrongByteBufferInputStream> mStream;
};