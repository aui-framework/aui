#pragma once

#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Common/AByteBuffer.h"

typedef struct OpusDecoder OpusDecoder;

/**
 * @brief Sound stream for OPUS format
 * @note Not intended for decoding OggOpus format
 */
class AOpusSoundStream : public ISoundInputStream {
public:
    static constexpr uint32_t SAMPLE_RATE = 48000;
    static constexpr uint8_t CHANNEL_COUNT = 2;

    explicit AOpusSoundStream(_<IInputStream> is);

    ~AOpusSoundStream();

    size_t read(char* dst, size_t size) override;

    AAudioFormat info() override;

    void rewind() override;

private:
    AByteBuffer mBuffer;
    OpusDecoder* mDecoder;
};