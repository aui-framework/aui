//
// Created by ilyazavalov on 11/1/23.
//

#pragma once

#include <cstdint>
#include "AUI/Audio/AAudioFormat.h"
#include "AUI/Traits/platform.h"

/**
 * @brief Default output format for the current platform.
 */
namespace aui::audio::platform {
    constexpr AChannelFormat requested_channels_format = AChannelFormat::STEREO;
    constexpr std::uint32_t requested_sample_rate = 44100;
    constexpr ASampleFormat requested_sample_format = aui::platform::current::is_mobile() ? ASampleFormat::I16 :
                                                                                            ASampleFormat::I24;

    constexpr AAudioFormat requested_format {
        .channelCount = requested_channels_format,
        .sampleRate = requested_sample_rate,
        .sampleFormat = requested_sample_format
    };
}
