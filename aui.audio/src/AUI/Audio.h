/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

/**
 * @defgroup audio aui::audio
 * @brief Audio playback library.
 * @details
 * <!-- aui:experimental -->
 *
 * `aui::audio` plays sounds through the native audio API of the platform: PulseAudio on Linux, DirectSound on Windows,
 * CoreAudio on macOS and iOS, Oboe on Android. On other platforms a stub player is used, which plays nothing.
 *
 * The central class is [IAudioPlayer]. It is created from a url (a file, an [aui-assets] resource, an http address) or
 * from your own [ISoundInputStream]:
 *
 * ```cpp
 * auto player = IAudioPlayer::fromUrl(":sound/click.ogg");
 * player->play();
 * ```
 *
 * WAV and Ogg Vorbis files are recognized automatically. Whatever the format of the source is, it is resampled and
 * converted to what the audio device wants, so sounds of different formats can be played at the same time. All players
 * that are playing at the moment are mixed together by [AAudioMixer].
 *
 * !!! warning
 *
 *     Recording (capturing audio from a microphone) and device enumeration are not implemented. The module only
 *     plays sound.
 *
 * - [audio-playback] covers playing files, controlling playback, volume and looping.
 * - [audio-custom-sources] describes how to play generated or decoded-by-you audio.
 */
