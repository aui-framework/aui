//
// Created by dervisdev on 2/9/2023.
//

#include "AWavSoundStream.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"
#include "AUI/Url/AUrl.h"
#include "AUI/Audio/ABadFormatException.h"

AWavSoundStream::AWavSoundStream(AUrl url) : mUrl(std::move(url)) {
    mStream = loadSourceInputStream(*mUrl);
    if (mStream == nullptr) {
        throw AException("Failed to get input source for wav file from {}"_format(mUrl->full()));
    }
    readHeader();
}

AWavSoundStream::AWavSoundStream(aui::non_null<_<IInputStream>> stream) : mStream(std::move(stream)) {
    readHeader();
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
        mStream = loadSourceInputStream(*mUrl);
        if (mStream) {
            readHeader();
        }
    }
}

size_t AWavSoundStream::read(char* dst, size_t size) {
    size -= size % (mHeader.byteRate / mHeader.sampleRate);
    auto remaining = mHeader.subchunk2Size - mChunkReadPos;
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
        throw aui::audio::ABadFormatException("not a wav file");
    }
}
