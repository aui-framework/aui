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
        .channelCount = CHANNEL_COUNT,
        .sampleRate = SAMPLE_RATE,
        .sampleFormat = ASampleFormat::I16
    };
}

void AOpusSoundStream::rewind() {

}
