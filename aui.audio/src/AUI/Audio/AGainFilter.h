/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <AUI/Audio/VolumeLevel.h>

/**
 * @brief Implements audio gain filtering.
 * @ingroup audio
 * @details
 * Scales samples by a factor derived from `aui::audio::VolumeLevel`. Used internally by IAudioPlayer; use
 * `IAudioPlayer::setVolume` instead.
 *
 * Gain changes are not applied instantly: the gain is moved linearly, frame by frame, from its current value to the
 * new one over the smoothing time (see setSmoothing). Without this, a stepwise volume change (e.g., a fade driven by a
 * timer) makes the waveform jump and is heard as zipper noise (crackling). The very first gain is applied at once.
 *
 * setVolume/setSmoothing may be called from any thread; process is called from the audio thread.
 */
class API_AUI_AUDIO AGainFilter final {
public:
    enum class Layout {
        INTERLEAVED, //!< frame-major: L R L R ...
        PLANAR,      //!< channel-major: L L L ... R R R ...
    };

    static constexpr std::chrono::milliseconds DEFAULT_SMOOTHING { 40 };

    void process(float* samples, size_t frames, size_t channels, Layout layout = Layout::INTERLEAVED);

    void setVolume(aui::audio::VolumeLevel volume) noexcept;

    /**
     * @brief Sets the time over which a gain change is smoothed. Zero applies gain changes instantly.
     */
    void setSmoothing(std::chrono::milliseconds smoothing) noexcept;

    void setSampleRate(size_t sampleRate) noexcept;

private:
    aui::audio::VolumeLevel mVolume = 256;

    // written by any thread
    std::atomic<float> mTargetGain = 1.0f;
    std::atomic<size_t> mSampleRate = 48000;
    std::atomic<std::chrono::milliseconds::rep> mSmoothingMs = DEFAULT_SMOOTHING.count();

    // audio thread only
    bool mPrimed = false;
    float mCurrentGain = 1.0f;
    float mRampTarget = 1.0f;
    float mRampStep = 0.0f;
    size_t mRampLeft = 0;
};
