#include "AOpusSoundStream.h"
#include <opus.h>
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"

AOpusSoundStream::AOpusSoundStream(_<IInputStream> stream) : mStream(std::move(stream)) {
    mStream->read(reinterpret_cast<char*>(&mHeader), HEADER_SIZE);
    if (std::memcmp(mHeader.signature, "OpusHead", 8) != 0) {
        throw AException("not an OPUS file");
    }

    int error;
    mDecoder = opus_decoder_create(SAMPLE_RATE, CHANNEL_COUNT, &error);
    if (error != OPUS_OK) {
        opus_decoder_destroy(mDecoder);
        throw AException("Failed to initialize opus decoder, error code: {}"_format(error));
    }
}

AOpusSoundStream::~AOpusSoundStream() {
    opus_decoder_destroy(mDecoder);
}

size_t AOpusSoundStream::read(char *dst, size_t size) {
    if (auto bytesRead = mDecodedSamples.read(dst, size)) {
        //ok, we have some samples to read
        return bytesRead;
    }

    //we need to decode next packet
    auto packetSize = mStream->read(mPacketBuffer, sizeof(mPacketBuffer));
    if (packetSize == 0) {
        //waiting for packet...
        return 0;
    }


    auto res = opus_decode(mDecoder,
                           reinterpret_cast<const unsigned char*>(mPacketBuffer), static_cast<opus_int32>(packetSize),
                           reinterpret_cast<opus_int16*>(mSampleBuffer), static_cast<int>(MAX_UNPACKED_SIZE / 4), 0);
    if (res > 0) {
        //we got new samples and can perform read()
        size_t decodedSize = 2 * res;
        size_t toWrite = std::min(decodedSize, size);
        std::memcpy(dst, mSampleBuffer, toWrite);

        //if some samples left, we save it for future read()
        mDecodedSamples.write(dst + toWrite, decodedSize - toWrite);

        return toWrite;
    }

    return 0;
}

AAudioFormat AOpusSoundStream::info() {
    return AAudioFormat {
        .channelCount = CHANNEL_COUNT,
        .sampleRate = SAMPLE_RATE,
        .sampleFormat = ASampleFormat::I16
    };
}

void AOpusSoundStream::rewind() {

}
