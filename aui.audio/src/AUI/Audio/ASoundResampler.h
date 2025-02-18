/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Audio/ISoundInputStream.h>
#include <AUI/Audio/ACompileTimeSoundResampler.h>
#include <AUI/Audio/VolumeLevel.h>

class API_AUI_AUDIO IAudioPlayer;

namespace aui::audio::impl {
    class ResamplerBase {
    public:
        virtual ~ResamplerBase() = default;
        virtual size_t resample(std::span<std::byte>, aui::audio::VolumeLevel volume) = 0;
    };
}

/**
 * @brief Implements audio mixing and resampling.
 * @ingroup audio
 * @details
 * @experimental
 */
class API_AUI_AUDIO ASoundResampler final : public ISoundInputStream {
public:
    using IInputStream::read;

    explicit ASoundResampler(_<ISoundInputStream> sourceStream) noexcept;

    size_t read(char* dst, size_t size) override;

    AAudioFormat info() override;

    void setVolume(aui::audio::VolumeLevel volume) noexcept;

private:
    _<ISoundInputStream> mSourceStream;
    _unique<aui::audio::impl::ResamplerBase> mResampler;
    /**
     * @brief Volume level, integer from 0 to 256, works linear
     */
    aui::audio::VolumeLevel mVolume = 256;
};