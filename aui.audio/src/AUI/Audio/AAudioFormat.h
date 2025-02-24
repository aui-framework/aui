#pragma once

#include "AUI/Audio/ASampleFormat.h"
#include "AUI/Audio/AChannelFormat.h"
#include "AUI/Common/AException.h"

/**
 * @brief Audio format descriptor.
 * @ingroup audio
 * @details
 * @experimental
 */
struct API_AUI_AUDIO AAudioFormat {
    /**
     * @brief Channel count.
     */
    AChannelFormat channelCount;

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


