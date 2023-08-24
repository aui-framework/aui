#pragma once

#include "AUI/Audio/Sound/ISoundStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Audio/Stream/FileStream.h"

class RawSoundStream : public ISoundStream {
public:
    RawSoundStream(AAudioFormat format, AByteBuffer data) : mFormat(std::move(format)),
                                                            mStream(_new<FileStream>(std::move(data))) { }

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
    _<FileStream> mStream;
};