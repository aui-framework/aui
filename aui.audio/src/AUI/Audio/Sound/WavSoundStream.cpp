//
// Created by dervisdev on 2/9/2023.
//

#include "WavSoundStream.h"

WavSoundStream::WavSoundStream(_<IFileStream> is) : mFis(std::move(is)) {
    mFis->read(reinterpret_cast<char*>(&mHeader), sizeof(mHeader));

    if (std::memcmp(mHeader.chunkID, "RIFF", 4) != 0 ||
        std::memcmp(mHeader.format, "WAVE", 4) != 0 ||
        std::memcmp(mHeader.subchunk1ID, "fmt ", 4) != 0) {
        throw std::runtime_error("not a wav file");
    }
}

AAudioFormat WavSoundStream::info() {
    return AAudioFormat {
        .bitRate = static_cast<unsigned int>(mHeader.byteRate * 2),
        .channelCount = static_cast<uint8_t>(mHeader.numChannels),
        .sampleRate = static_cast<unsigned int>(mHeader.sampleRate),
        .bitsPerSample = static_cast<uint8_t>(mHeader.bitsPerSample)
    };
}

void WavSoundStream::rewind() {
    mChunkReadPos = 0;
    mFis->seek(sizeof(mHeader), std::ios::beg);
}

size_t WavSoundStream::read(char* dst, size_t size) {
    auto remaining = mHeader.chunkSize - mChunkReadPos;
    if (remaining == 0) {
        return 0;
    }

    size_t r = mFis->read(dst, std::min(size, remaining));
    mChunkReadPos += r;
    return r;
}

_<ISoundStream> WavSoundStream::load(_<IFileStream> is) {
    return _new<WavSoundStream>(std::move(is));
}