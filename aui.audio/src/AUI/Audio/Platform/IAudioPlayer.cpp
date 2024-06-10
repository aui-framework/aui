#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Audio/ISoundInputStream.h"
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
#endif

_<IAudioPlayer> IAudioPlayer::fromUrl(AUrl url) {
    return _new<DefaultSystemPlayer>(std::move(url));
}

IAudioPlayer::IAudioPlayer(AUrl url) : mUrl(std::move(url)) {
    initialize();
}

void IAudioPlayer::initialize() {
    mSourceStream = ISoundInputStream::fromUrl(mUrl);
    mResampledStream = _new<ASoundResampler>(mSourceStream);
    mResampledStream->setVolume(mVolume);
}

void IAudioPlayer::play() {
    if (mPlaybackStatus != PlaybackStatus::PLAYING) {
        playImpl();
        mPlaybackStatus = PlaybackStatus::PLAYING;
    }
}

void IAudioPlayer::pause() {
    if (mPlaybackStatus == PlaybackStatus::PLAYING) {
        pauseImpl();
        mPlaybackStatus = PlaybackStatus::PAUSED;
    }
}

void IAudioPlayer::stop() {
    if (mPlaybackStatus != PlaybackStatus::STOPPED) {
        stopImpl();
        mPlaybackStatus = PlaybackStatus::STOPPED;
    }
}

void IAudioPlayer::release() {
    mResampledStream.reset();
    mSourceStream.reset();
}
