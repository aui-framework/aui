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

#include <AUI/Audio/VolumeLevel.h>

/**
 * @brief Implements audio gain filtering.
 * @ingroup audio
 * @details
 */
class API_AUI_AUDIO AGainFilter final {
public:
    void process(float* samples, size_t num_samples);

    void setVolume(aui::audio::VolumeLevel volume) noexcept;

private:
    /**
     * @brief Volume level, integer from 0 to 256, works linear
     */
    aui::audio::VolumeLevel mVolume = 256;
    float mGain = 1.0f;
};
