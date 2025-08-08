#include "AUI/Audio/IAudioPlayer.h"

#include "RequestedAudioFormat.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Audio/StubAudioPlayer.h"
#include "AUI/Url/AUrl.h"

#if AUI_PLATFORM_WIN
#include "win32/DirectSoundAudioPlayer.h"
using DefaultSystemPlayer = DirectSoundAudioPlayer;
#elif AUI_PLATFORM_LINUX
#include "linux/PulseAudioPlayer.h"
using DefaultSystemPlayer = PulseAudioPlayer;
#elif AUI_PLATFORM_ANDROID
#include "android/OboeAudioPlayer.h"
using DefaultSystemPlayer = OboeAudioPlayer;
#elif AUI_PLATFORM_APPLE
#include "apple/CoreAudioPlayer.h"
using DefaultSystemPlayer = CoreAudioPlayer;
#else
using DefaultSystemPlayer = StubAudioPlayer;
#endif

_<IAudioPlayer> IAudioPlayer::fromUrl(AUrl url) {
    return _new<DefaultSystemPlayer>(std::move(url));
}

_<IAudioPlayer> IAudioPlayer::fromStream(_<ISoundInputStream> stream) {
    return _new<DefaultSystemPlayer>(std::move(stream));
}

IAudioPlayer::IAudioPlayer(AUrl url) : mUrl(std::move(url)) {
    initialize();
}

IAudioPlayer::IAudioPlayer(_<ISoundInputStream> stream) {
    mSourceStream = std::move(stream);
    mResamplerStream.emplace(aui::audio::platform::requested_sample_rate, mSourceStream);
    mResamplerStream->setVolume(mVolume);
}

void IAudioPlayer::initialize() {
    if (!mUrl) {
        throw AException("url is empty");
    }
    mSourceStream = ISoundInputStream::fromUrl(*mUrl);
    mResamplerStream.emplace(aui::audio::platform::requested_sample_rate, mSourceStream);
    mResamplerStream->setVolume(mVolume);
}

void IAudioPlayer::play() {
    if (mPlaybackStatus != PlaybackStatus::PLAYING) {
        mPlaybackStatus = PlaybackStatus::PLAYING;
        playImpl();
    }
}

void IAudioPlayer::pause() {
    if (mPlaybackStatus == PlaybackStatus::PLAYING) {
        mPlaybackStatus = PlaybackStatus::PAUSED;
        pauseImpl();
    }
}

void IAudioPlayer::stop() {
    if (mPlaybackStatus != PlaybackStatus::STOPPED) {
        mPlaybackStatus = PlaybackStatus::STOPPED;
        stopImpl();
    }
}

void IAudioPlayer::reset() {
    mResamplerStream.reset();
    mSourceStream.reset();
}

void IAudioPlayer::setLoop(bool loop) {
    mLoop = loop;
    onLoopSet();
}

void IAudioPlayer::setVolume(aui::audio::VolumeLevel volume) {
    mVolume = volume;
    AUI_NULLSAFE(mResamplerStream)->setVolume(volume);
    onVolumeSet();
}

void IAudioPlayer::onFinished() {
    reset();
    mPlaybackStatus = PlaybackStatus::STOPPED;
    getThread()->enqueue([this, self = _cast<IAudioPlayer>(aui::ptr::shared_from_this(this))]() {
        emit finished;
    });
}

void IAudioPlayer::rewind() {
    reset();
    initialize();
}
