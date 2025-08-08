//
// Created by dervisdev on 2/9/2023.
//

#include "AWavSoundStream.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"
#include "AUI/Url/AUrl.h"
#include "AUI/Audio/ABadFormatException.h"

AWavSoundStream::AWavSoundStream(aui::non_null<_unique<IInputStream>> stream) : mStream(std::move(stream.value)) {
    readHeader();
}

AAudioFormat AWavSoundStream::info() {
    return AAudioFormat {
        .channelCount = static_cast<AChannelFormat>(mHeader.numChannels),
        .sampleRate = static_cast<uint32_t>(mHeader.sampleRate),
        .sampleFormat = static_cast<uint8_t>(mHeader.bitsPerSample) == 24 ? ASampleFormat::I24 : ASampleFormat::I16,
    };
}

size_t AWavSoundStream::read(char* dst, size_t size) {
    size -= size % (mHeader.byteRate / mHeader.sampleRate);
    size_t r = mStream->read(dst, size);
    mChunkReadPos += r;
    return r;
}

void AWavSoundStream::readHeader() {
    mStream->read(reinterpret_cast<char*>(&mHeader), sizeof(mHeader));
    if (std::memcmp(mHeader.chunkID, "RIFF", 4) != 0 ||
        std::memcmp(mHeader.format, "WAVE", 4) != 0 ||
        std::memcmp(mHeader.subchunk1ID, "fmt ", 4) != 0) {
        throw aui::audio::ABadFormatException("not a wav file");
    }
}
