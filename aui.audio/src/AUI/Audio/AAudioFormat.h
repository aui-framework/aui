#pragma once

#include "AUI/Audio/ASampleFormat.h"
#include "AUI/Common/AException.h"

/**
 * @brief Audio format descriptor.
 * @ingroup audio
 */
struct API_AUI_AUDIO AAudioFormat {
    /**
     * @Brief Bits per second.
     */
    uint32_t bitRate;

    /**
     * @brief Channel count.
     */
    std::uint8_t channelCount;

    /**
     * @brief Samples count per second (i.e. 48000, 44100).
     */
    std::uint32_t sampleRate;

    ASampleFormat sampleFormat;

    /**
     * @brief Bits per sample (i.e. 16, 8).
     */
    [[nodiscard]]
    std::uint32_t bitsPerSample() const;
};


