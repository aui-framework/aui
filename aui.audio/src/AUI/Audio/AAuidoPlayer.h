#pragma once

#include "AUI/Audio/Sound/ISoundStream.h"
#include "AUI/Audio/Mixer/AAudioMixer.h"

#if AUI_PLATFORM_ANDROID
#include "AUI/Audio/Mixer/ASampleCommitter.h"
#endif

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
        if (mPlaybackStatus != PlaybackStatus::PLAYING) {
            playImpl();
            mPlaybackStatus = PlaybackStatus::PLAYING;
        }
    }

    void pause() {
        if (mPlaybackStatus == PlaybackStatus::PLAYING) {
            pauseImpl();
            mPlaybackStatus = PlaybackStatus::PAUSED;
        }
    }

    void stop() {
        if (mPlaybackStatus != PlaybackStatus::STOPPED) {
            stopImpl();
            mPlaybackStatus = PlaybackStatus::STOPPED;
        }
    }

    PlaybackStatus getPlaybackStatus() {
        return mPlaybackStatus;
    }

    void setSource(_<ISoundStream> src) {
        if (mPlaybackStatus != PlaybackStatus::STOPPED) {
            stop();
        }
        mSource = std::move(src);
#if AUI_PLATFORM_ANDROID
        mCommitter = _new<ASampleCommitter>(mSource);
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
    _<ASampleCommitter> mCommitter;
#endif

    void playImpl();

    void pauseImpl();

    void stopImpl();

};
