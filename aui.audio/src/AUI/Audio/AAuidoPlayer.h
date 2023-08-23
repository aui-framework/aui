#pragma once

#include "AUI/Audio/Sound/ISoundStream.h"
#include "AUI/Audio/Mixer/AAudioMixer.h"

class AAudioPlayer {
public:
    AAudioPlayer() = default;

    explicit AAudioPlayer(_<ISoundStream> stream) {
        setSource(std::move(stream));
    }

    enum class PlaybackStatus : int8_t {
        PLAYING,
        PAUSED,
        STOPPED
    };

    void play() {
        playImpl();
        mPlaybackStatus = PlaybackStatus::PLAYING;
    }

    void pause() {
        pauseImpl();
        mPlaybackStatus = PlaybackStatus::PAUSED;
    }

    void stop() {
        stopImpl();
        mPlaybackStatus = PlaybackStatus::STOPPED;
    }

    PlaybackStatus getPlaybackStatus() {
        return mPlaybackStatus;
    }

    void setSource(_<ISoundStream> src) {
        stop();
        mSource = std::move(src);
#if AUI_PLATFORM_ANDROID
        mCommitter = _new<SampleCommitter>(mSource);
#endif
    }

    void setLoop(bool loop) {
        mLoop = loop;
    }

    void setVolume(float volume) {
        mVolume = volume;
    }

private:
    _<ISoundStream> mSource;
    PlaybackStatus mPlaybackStatus = PlaybackStatus::STOPPED;
    bool mLoop = false;
    float mVolume = 1.f;

#if AUI_PLATFORM_ANDROID
    _<SampleCommitter> mCommitter;
#endif

    void playImpl();

    void pauseImpl();

    void stopImpl();

};
