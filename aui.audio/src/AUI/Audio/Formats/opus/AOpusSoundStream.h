#pragma once

#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Common/AByteBuffer.h"
#include "AUI/IO/APortionedMultipleStream.h"
#include "AUI/IO/APipe.h"

typedef struct OpusDecoder OpusDecoder;

/**
 * @brief Sound stream for OPUS format
 * @note Not intended for decoding OggOpus format
 */
class AOpusSoundStream : public ISoundInputStream {
public:
    static constexpr uint32_t SAMPLE_RATE = 48000;
    static constexpr uint8_t CHANNEL_COUNT = 2;
    static constexpr ASampleFormat SAMPLE_FORMAT = ASampleFormat::I16;

    explicit AOpusSoundStream(_<IInputStream> stream);

    ~AOpusSoundStream();

    size_t read(char* dst, size_t size) override;

    AAudioFormat info() override;

    void rewind() override;

private:
    static constexpr size_t HEADER_SIZE = 18;
    static constexpr size_t PACKET_BUFFER_SIZE = 8192;
    static constexpr size_t MAX_UNPACKED_SIZE = 11520;

    struct OpusHead {
        char signature[8];
        uint8_t version;
        uint8_t outputChannelCount;
        uint16_t preSkip; //TODO implement preskipping first samples
        uint32_t inputSampleRate;
        int16_t outputGain;
        char _pad[2];
    };

    static_assert(sizeof(OpusHead) == 20);


    _<IInputStream> mStream;
    APortionedMultipleStream mSamples;
    OpusHead mHeader;
    char mPacketBuffer[PACKET_BUFFER_SIZE];
    AByteBuffer mBuffer;
    OpusDecoder* mDecoder;
};