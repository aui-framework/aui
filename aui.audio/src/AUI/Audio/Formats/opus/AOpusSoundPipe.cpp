#include "AOpusSoundPipe.h"
#include <opus.h>
#include "AUI/Common/AByteBuffer.h"
#include "AUI/Logging/ALogger.h"
#include "AUI/IO/AStrongByteBufferInputStream.h"

static constexpr auto LOG_TAG = "opus";

AOpusSoundPipe::~AOpusSoundPipe() {
    opus_decoder_destroy(mDecoder);
}

void AOpusSoundPipe::write(const char *src, size_t size) {
    std::unique_lock lock(mSync);
    if (!mHeaderHasRead) {
        std::memcpy(reinterpret_cast<char*>(&mHeader), src, HEADER_SIZE);
        if (std::memcmp(mHeader.signature, "OpusHead", 8) != 0) {
            ALogger::warn(LOG_TAG) << "A non-opus header was provided to AOpusSoundStream, waiting for header...";
            mLastWriteSuccessful = false;
            return;
        }

        int error;
        mDecoder = opus_decoder_create(SAMPLE_RATE, static_cast<int>(CHANNEL_COUNT), &error);
        if (error != OPUS_OK) {
            opus_decoder_destroy(mDecoder);
            ALogger::warn(LOG_TAG) << "Failed to create decoder";
            mLastWriteSuccessful = false;
            return;
        }

        mHeaderHasRead = true;
        mLastWriteSuccessful = true;
        return;
    }

    auto res = opus_decode(mDecoder,
                           reinterpret_cast<const unsigned char*>(src), static_cast<opus_int32>(size),
                           reinterpret_cast<opus_int16*>(mSampleBuffer), static_cast<int>(MAX_UNPACKED_SIZE / 4),
                           0);
    mLastWriteSuccessful = res > 0;
    if (res > 0) {
        mDecodedSamples.write(mSampleBuffer, res * 4);
    }

}

size_t AOpusSoundPipe::read(char *dst, size_t size) {
    return mDecodedSamples.read(dst, size);
}

AAudioFormat AOpusSoundPipe::info() {
    return AAudioFormat {
        .channelCount = CHANNEL_COUNT,
        .sampleRate = SAMPLE_RATE,
        .sampleFormat = ASampleFormat::I16
    };
}
