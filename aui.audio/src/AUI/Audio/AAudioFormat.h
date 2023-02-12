#pragma once

#include <AUI/Common/AObject.h>

/**
 * @brief Audio format descriptor.
 * @ingroup audio
 */
struct AAudioFormat {
    /**
     * @Brief Bits per second
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

    /**
     * @brief Bits per sample (i.e. 16, 8).
     */
    std::uint32_t bitsPerSample;
};


