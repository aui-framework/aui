#pragma once

#include "AUI/Audio/Sound/ISoundStream.h"

class AAudioPlayer {
public:

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
        mSource = std::move(src);
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

    void playImpl();

    void pauseImpl();

    void stopImpl();

};
