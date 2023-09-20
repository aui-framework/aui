#include "AOpusSoundStream.h"
#include <opus.h>
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Logging/ALogger.h"

AOpusSoundStream::AOpusSoundStream(_<IInputStream> is) {
    int error;
    mDecoder = opus_decoder_create(SAMPLE_RATE, CHANNEL_COUNT, &error);
    if (error != OPUS_OK) {
        opus_decoder_destroy(mDecoder);
        throw AException("Failed to initialize opus decoder, error code: {}"_format(error));
    }

    auto buffer = AByteBuffer::fromStream(is, 4096);
    opus_int16 decoded[2 * 5760];
    auto samples = opus_decode(mDecoder,
                               reinterpret_cast<const unsigned char*>(buffer.data()), static_cast<opus_int32>(buffer.size()),
                               decoded, sizeof(decoded), 0);
    ALogger::info("opus") << "opus decode: " << samples;
    mBuffer.resize(samples * 4);
    std::memcpy(mBuffer.data(), reinterpret_cast<char*>(decoded), mBuffer.size());

}

AOpusSoundStream::~AOpusSoundStream() {
    opus_decoder_destroy(mDecoder);
}

size_t AOpusSoundStream::read(char *dst, size_t size) {
    size = std::min(size, mBuffer.size());
    std::memcpy(dst, mBuffer.data(), size);
    mBuffer.clear();
    return size;
}

AAudioFormat AOpusSoundStream::info() {
    return AAudioFormat {
        .bitRate = 16 * SAMPLE_RATE,
        .channelCount = CHANNEL_COUNT,
        .sampleRate = SAMPLE_RATE,
        .sampleFormat = ASampleFormat::I16
    };
}

void AOpusSoundStream::rewind() {

}
