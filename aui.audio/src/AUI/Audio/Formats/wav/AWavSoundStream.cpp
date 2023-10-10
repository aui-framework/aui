//
// Created by dervisdev on 2/9/2023.
//

#include "AWavSoundStream.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"
#include "AUI/Url/AUrl.h"

AWavSoundStream::AWavSoundStream(AUrl url) : mUrl(std::move(url)) {
    mStream = mUrl->open();
    readHeader();
}

AWavSoundStream::AWavSoundStream(_<IInputStream> stream) : mStream(std::move(stream)) {

}

AAudioFormat AWavSoundStream::info() {
    return AAudioFormat {
        .channelCount = static_cast<AChannelFormat>(mHeader.numChannels),
        .sampleRate = static_cast<unsigned int>(mHeader.sampleRate),
        .sampleFormat = static_cast<uint8_t>(mHeader.bitsPerSample) == 24 ? ASampleFormat::I24 : ASampleFormat::I16,
    };
}

void AWavSoundStream::rewind() {
    if (mUrl) {
        mChunkReadPos = 0;
        mStream.reset();
        mStream = mUrl->open();
        if (mStream) {
            readHeader();
        }
    }
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

_<AWavSoundStream> AWavSoundStream::fromUrl(AUrl url) {
    return _new<AWavSoundStream>(std::move(url));
}

void AWavSoundStream::readHeader() {
    mStream->read(reinterpret_cast<char*>(&mHeader), sizeof(mHeader));
    if (std::memcmp(mHeader.chunkID, "RIFF", 4) != 0 ||
        std::memcmp(mHeader.format, "WAVE", 4) != 0 ||
        std::memcmp(mHeader.subchunk1ID, "fmt ", 4) != 0) {
        throw AException("not a wav file");
    }
}
