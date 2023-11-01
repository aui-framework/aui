//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Audio/ACompileTimeSoundResampler.h"

class IAudioPlayer;

/**
 * @brief Implements audio mixing and resampling.
 * @ingroup audio
 */
class API_AUI_AUDIO ASoundResampler : public ISoundInputStream {
public:
    static constexpr AAudioFormat DEFAULT_OUTPUT_FORMAT = {
            .channelCount = aui::audio::DEFAULT_OUTPUT_CHANNELS_COUNT,
            .sampleRate = aui::audio::DEFAULT_OUTPUT_SAMPLE_RATE,
            .sampleFormat = aui::audio::DEFAULT_OUTPUT_SAMPLE_FORMAT
    };

    explicit ASoundResampler(const _<IAudioPlayer>& player) noexcept;

    //unimplemented
    //explicit ASoundResampler(const _<IAudioPlayer>& player, AAudioFormat format) noexcept;

    size_t read(char* dst, size_t size) override;

    AAudioFormat info() override;

    void rewind() override;

private:
    _weak<IAudioPlayer> mParentPlayer;
    _<ISoundInputStream> mSoundStream;
//    AAudioFormat mOutputFormat;
    AAudioFormat mInputFormat;

    template<ASampleFormat inputSampleFormat, AChannelFormat inputChannelsFormat>
    size_t commitSamples(std::span<std::byte> dst);
};