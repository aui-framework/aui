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

#include "AUI/Common/AException.h"

/**
 * @brief Supported channels formats  (mono, stereo etc.)
 */
enum AChannelFormat : uint8_t {
    /**
     * @brief Mono sound
     */
    MONO = 1,
    /**
     * @brief Stereo sound
     */
    STEREO = 2,
};

/**
 * @brief Sample formats supported for mixing.
 * @ingroup audio
 */
enum class ASampleFormat {
    /**
     * @brief Signed 16-bit integer.
     */
    I16,

    /**
     * @brief Signed 24-bit integer.
     */
    I24,

    /**
     * @brief Signed 32-bit integer.
     */
    I32,

    /**
     * @brief 32-bit IEEE floating point.
     */
    F32,
};

namespace aui::audio {
constexpr size_t bytesPerSample(ASampleFormat format) {
    switch (format) {
        case ASampleFormat::I16:
            return 2;

        case ASampleFormat::I24:
            return 3;

        case ASampleFormat::F32:
        case ASampleFormat::I32:
            return 4;
    }

    throw AException("Wrong sample format");
}
void convertSampleFormat(ASampleFormat inF, ASampleFormat outF, const char* inB, char* outB, size_t samples);
}

/**
 * @brief Audio format descriptor.
 * @ingroup audio
 * @details
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
};
