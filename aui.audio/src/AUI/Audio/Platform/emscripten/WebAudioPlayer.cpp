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

    // Module.auiAudio stays the same object (pages keep it, e.g. for Media Session play/pause); its ctx, node and gain
    // are replaced when the context has to be created anew
    const audio = Module.auiAudio = { ctx: null, node: null, gain: null };
    const log = (what) => console.log('[WebAudio] ' + what + ': ' + audio.ctx.state + ', ' + audio.ctx.sampleRate + ' Hz');
    let unlocked = false;
    const create = () => {
        if (audio.ctx) {
            audio.node.onaudioprocess = null;
            audio.ctx.onstatechange = null;
            audio.ctx.close().catch(() => {});
        }
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
        // through a gain: the sound fades out before the context is suspended and in after it resumes, a hard stop
        // mid-buffer cracks
        const gain = ctx.createGain();
        node.connect(gain);
        gain.connect(ctx.destination);
        audio.ctx = ctx;
        audio.node = node;
        audio.gain = gain;
        unlocked = false;   // a new context is unlocked by a gesture of its own
        log('created');
        ctx.onstatechange = () => log('state');
    };
    create();

    // iOS: Web Audio obeys the ring/silent switch (media does not), so with the switch on silent there is no sound at
    // all. 'session': Safari 17+ lets the page declare itself media (navigator.audioSession); 'element': older iOS
    // switches to the media session while an <audio> element plays, so a silent one loops (Safari may show it as Now
    // Playing); 'off': obey the switch. The page may choose with Module.auiAudioSession; by default the session where
    // there is one, else nothing
    const isIos = /iPhone|iPad|iPod/.test(navigator.userAgent) || (navigator.maxTouchPoints > 1 && /Macintosh/.test(navigator.userAgent));
    const mode = Module.auiAudioSession || (navigator.audioSession ? 'session' : 'off');
    console.log('[WebAudio] ios: ' + isIos + ', navigator.audioSession: ' + typeof navigator.audioSession + ', mode: ' + mode);
    const declareSession = () => {
        if (mode !== 'session' || !navigator.audioSession) return;
        try { navigator.audioSession.type = 'playback'; } catch (e) { console.log('[WebAudio] audioSession: ' + e); }
    };
    declareSession();
    let silentLoop = null;
    if (mode === 'element') {
        silentLoop = document.createElement('audio');
        silentLoop.setAttribute('x-webkit-airplay', 'deny');
        silentLoop.preload = 'auto';
        silentLoop.loop = true;
        // 0.1 s of 8 kHz 8-bit mono silence
        const samples = 800;
        const wav = new Uint8Array(44 + samples);
        const view = new DataView(wav.buffer);
        const text = (offset, str) => { for (let i = 0; i < str.length; ++i) wav[offset + i] = str.charCodeAt(i); };
        text(0, 'RIFF'); view.setUint32(4, 36 + samples, true); text(8, 'WAVEfmt ');
        view.setUint32(16, 16, true); view.setUint16(20, 1, true); view.setUint16(22, 1, true);
        view.setUint32(24, 8000, true); view.setUint32(28, 8000, true); view.setUint16(32, 1, true); view.setUint16(34, 8, true);
        text(36, 'data'); view.setUint32(40, samples, true); wav.fill(128, 44);
        silentLoop.src = URL.createObjectURL(new Blob([wav], { type: 'audio/wav' }));
    }

    const FADE = 0.015;   // s, time constant of the fades (~50 ms to silence)
    const fadeTo = (value) => {
        const g = audio.gain.gain;
        const now = audio.ctx.currentTime;
        g.cancelScheduledValues(now);
        g.setValueAtTime(g.value, now);
        g.setTargetAtTime(value, now, FADE);
    };

    // iOS Safari leaves a context that was put to background 'interrupted' (or 'suspended' with resume() never
    // settling): it does not come back by itself, so a context that is not running some time after resume() is replaced
    let staleTimer = null;
    const checkRunning = () => {
        clearTimeout(staleTimer);
        staleTimer = setTimeout(() => {
            if (document.hidden || audio.ctx.state === 'running') return;
            console.log('[WebAudio] the context did not resume (' + audio.ctx.state + '): creating a new one');
            create();
            resume();
        }, 400);
    };

    // browsers start the context only from a user gesture; older iOS also wants a sound started inside that gesture
    const resume = () => {
        if (document.hidden) return;
        if (audio.ctx.state === 'closed' || audio.ctx.state === 'interrupted') create();
        if (silentLoop && silentLoop.paused) silentLoop.play().catch(() => {});
        if (audio.ctx.state !== 'running') {
            declareSession();
            audio.gain.gain.setValueAtTime(0, audio.ctx.currentTime);
            audio.ctx.resume().catch(() => {});
            checkRunning();
        }
        fadeTo(1);
        if (!unlocked) {
            unlocked = true;
            const source = audio.ctx.createBufferSource();
            source.buffer = audio.ctx.createBuffer(1, 1, audio.ctx.sampleRate);
            source.connect(audio.ctx.destination);
            source.start(0);
        }
    };
    for (const type of ['pointerdown', 'pointerup', 'touchstart', 'touchend', 'mousedown', 'keydown', 'click']) {
        document.addEventListener(type, resume, { capture: true, passive: true });
    }
    let suspendTimer = null;
    const hide = () => {
        clearTimeout(staleTimer);
        if (silentLoop) silentLoop.pause();
        if (audio.ctx.state !== 'running') return;
        // fade out first, suspend once it is silent
        fadeTo(0);
        clearTimeout(suspendTimer);
        suspendTimer = setTimeout(() => {
            if (document.hidden) audio.ctx.suspend().catch(() => {});
        }, FADE * 5 * 1000);
    };
    document.addEventListener('visibilitychange', () => {
        if (document.hidden) hide();
        else resume();
    });
    window.addEventListener('pagehide', hide);
    // back from the bfcache or from the app switcher: visibilitychange may not come on iOS
    window.addEventListener('pageshow', resume);
    window.addEventListener('focus', resume);
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
