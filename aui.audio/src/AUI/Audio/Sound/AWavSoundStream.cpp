//
// Created by dervisdev on 2/9/2023.
//

#include "AWavSoundStream.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"
#include "AUI/Url/AUrl.h"

AWavSoundStream::AWavSoundStream(_<ISeekableInputStream> is) : mStream(std::move(is)) {
    mStream->read(reinterpret_cast<char*>(&mHeader), sizeof(mHeader));

    if (std::memcmp(mHeader.chunkID, "RIFF", 4) != 0 ||
        std::memcmp(mHeader.format, "WAVE", 4) != 0 ||
        std::memcmp(mHeader.subchunk1ID, "fmt ", 4) != 0) {
        throw std::runtime_error("not a wav file");
    }
}

AAudioFormat AWavSoundStream::info() {
    return AAudioFormat {
        .bitRate = static_cast<unsigned int>(mHeader.byteRate * 2),
        .channelCount = static_cast<uint8_t>(mHeader.numChannels),
        .sampleRate = static_cast<unsigned int>(mHeader.sampleRate),
        .bitsPerSample = static_cast<uint8_t>(mHeader.bitsPerSample)
    };
}

void AWavSoundStream::rewind() {
    mChunkReadPos = 0;
    mStream->seek(sizeof(mHeader), std::ios::beg);
}

size_t AWavSoundStream::read(char* dst, size_t size) {
    auto remaining = mHeader.chunkSize - mChunkReadPos;
    if (remaining == 0) {
        return 0;
    }

    size_t r = mStream->read(dst, std::min(size, remaining));
    mChunkReadPos += r;
    return r;
}

_<ISoundStream> AWavSoundStream::load(_<ISeekableInputStream> is) {
    return _new<AWavSoundStream>(std::move(is));
}

_<AWavSoundStream> AWavSoundStream::fromUrl(const AUrl& url) {
    return _new<AWavSoundStream>(AStrongByteBufferInputStream::fromUrl(url));
}
