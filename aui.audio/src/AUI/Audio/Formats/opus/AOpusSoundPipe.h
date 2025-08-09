#pragma once

#include "AUI/Audio/ISoundPipe.h"
#include "AUI/IO/ADynamicPipe.h"

typedef struct OpusDecoder OpusDecoder;

/**
 * @brief Sound pipe for OPUS format, decodes OPUS packets
 * @details
 * !!! note
 *
 *     Ready packets should be passed with write() method for proper work, header must precede the packets.
 */
class AOpusSoundPipe : public ISoundPipe {
public:
    static constexpr uint32_t SAMPLE_RATE = 48000;
    static constexpr AChannelFormat CHANNEL_COUNT = AChannelFormat::STEREO;
    static constexpr ASampleFormat SAMPLE_FORMAT = ASampleFormat::I16;

    ~AOpusSoundPipe() override;

    void write(const char *src, size_t size) override;

    size_t read(char* dst, size_t size) override;

    AAudioFormat info() override;

    [[nodiscard]]
    bool isLastWriteSuccessful() const {
        return mLastWriteSuccessful;
    }

private:
    static constexpr size_t HEADER_SIZE = 19;
    static constexpr size_t MAX_UNPACKED_SIZE = 11520;

    struct OpusHead {
        char signature[8];
        uint8_t version;
        uint8_t outputChannelCount;
        uint16_t preSkip; //TODO implement preskipping first samples
        uint32_t inputSampleRate;
        int16_t outputGain;
        int8_t channelMap;
        char _pad[1];
    };

    static_assert(sizeof(OpusHead) == 20);


    ADynamicPipe mDecodedSamples;
    bool mHeaderHasRead = false;
    bool mLastWriteSuccessful = true;

    OpusHead mHeader;
    OpusDecoder* mDecoder;
    char mSampleBuffer[MAX_UNPACKED_SIZE];
    size_t mPreSkipped = 0;
    AMutex mSync;
};