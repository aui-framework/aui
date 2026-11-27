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

#include "AUI/Audio/IAudioPlayer.h"

/**
 * @brief Plays through Web Audio: one AudioContext for the whole page; its ScriptProcessorNode pulls the mixed samples
 * of all players from AAudioMixer on the main thread (no pthreads/SharedArrayBuffer needed).
 * @details
 * Browsers keep an AudioContext suspended until the first user gesture (autoplay policy); the context is resumed on the
 * first pointer/key event, players started before that are heard from that moment. The context is suspended while the
 * page is hidden.
 */
class WebAudioPlayer : public IAudioPlayer {
public:
    using IAudioPlayer::IAudioPlayer;

private:
    void playImpl() override;
    void pauseImpl() override;
    void stopImpl() override;

    void onLoopSet() override;
    void onVolumeSet() override;
};
