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
#include "WebAudioPlayer.h"

#include <emscripten.h>
#include <span>

#include "AUI/Audio/AAudioMixer.h"
#include "AUI/Audio/Platform/RequestedAudioFormat.h"
#include "AUI/Common/AVector.h"

namespace {
/// ~93 ms at 44.1 kHz: big enough for slow phones, small enough for UI sounds
constexpr int FRAMES_PER_CALLBACK = 4096;
constexpr int CHANNELS = 2;
static_assert(aui::audio::platform::requested_channels_format == AChannelFormat::STEREO);
static_assert(aui::audio::platform::requested_sample_format == ASampleFormat::F32);

AAudioMixer& mixer() {
    static AAudioMixer m;
    return m;
}

AVector<float>& renderBuffer() {
    static AVector<float> b;
    return b;
}
}   // namespace

/// Called from the ScriptProcessorNode: interleaved stereo floats, `frames` frames.
extern "C" EMSCRIPTEN_KEEPALIVE float* aui_webaudio_render(int frames) {
    auto& buffer = renderBuffer();
    buffer.resize(std::size_t(frames) * CHANNELS);
    mixer().readSoundData(std::as_writable_bytes(std::span(buffer)));
    return buffer.data();
}

// clang-format off
EM_JS(void, aui_webaudio_start, (int sampleRate, int frames), {
    if (Module.auiAudio) return;
    const AudioContextImpl = window.AudioContext || window.webkitAudioContext;
    if (!AudioContextImpl) return;
    let ctx;
    try {
        ctx = new AudioContextImpl({ sampleRate: sampleRate, latencyHint: 'interactive' });
    } catch (e) {
        ctx = new AudioContextImpl();   // older Safari: no options; the browser resamples
    }
    const node = ctx.createScriptProcessor(frames, 0, 2);
    node.onaudioprocess = (e) => {
        const out = e.outputBuffer;
        const n = out.length;
        const ptr = _aui_webaudio_render(n) >> 2;
        const left = out.getChannelData(0);
        const right = out.getChannelData(1);
        const heap = HEAPF32;
        for (let i = 0; i < n; ++i) {
            left[i] = heap[ptr + 2 * i];
            right[i] = heap[ptr + 2 * i + 1];
        }
    };
    node.connect(ctx.destination);
    Module.auiAudio = { ctx: ctx, node: node };

    const resume = () => { if (ctx.state !== 'running' && !document.hidden) ctx.resume(); };
    for (const type of ['pointerdown', 'pointerup', 'touchstart', 'touchend', 'mousedown', 'keydown', 'click']) {
        document.addEventListener(type, resume, { capture: true, passive: true });
    }
    document.addEventListener('visibilitychange', () => {
        if (document.hidden) ctx.suspend(); else resume();
    });
    resume();
});
// clang-format on

void WebAudioPlayer::playImpl() {
    aui_webaudio_start(int(aui::audio::platform::requested_sample_rate), FRAMES_PER_CALLBACK);
    initializeIfNeeded();
    mixer().addSoundSource(aui::ptr::shared_from_this(this));
}

void WebAudioPlayer::pauseImpl() { mixer().removeSoundSource(aui::ptr::shared_from_this(this)); }

void WebAudioPlayer::stopImpl() {
    mixer().removeSoundSource(aui::ptr::shared_from_this(this));
    reset();
}

void WebAudioPlayer::onLoopSet() {}

void WebAudioPlayer::onVolumeSet() {}
