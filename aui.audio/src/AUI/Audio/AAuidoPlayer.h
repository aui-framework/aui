#pragma once

#include "AUI/Audio/Sound/ISoundStream.h"
#include "AUI/Audio/Mixer/AAudioMixer.h"

#if AUI_PLATFORM_WIN
#include "AUI/Audio/Platform/win32/DirectSound.h"
#elif AUI_PLATFORM_ANDROID
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

#if AUI_PLATFORM_WIN
    static constexpr int BUFFER_DURATION_SEC = 2;
    static_assert(BUFFER_DURATION_SEC >= 2 && "Buffer duration assumes to be greater than 1");

    HANDLE mEvents[BUFFER_DURATION_SEC + 1];
    DSBPOSITIONNOTIFY mNotifyPositions[BUFFER_DURATION_SEC];
    HANDLE mThread;
    bool mThreadIsActive = false;

    IDirectSoundBuffer8* mSoundBufferInterface;
    IDirectSoundNotify8* mNotifyInterface;

    bool mIsPlaying = false;
    int mBytesPerSecond;

    void uploadNextBlock(DWORD reachedPointIndex);

    void clearBuffer();

    void setupBufferThread();

    [[noreturn]]
    static DWORD WINAPI bufferThread(void *lpParameter);

    void onAudioReachCallbackPoint();

    void setupReachPointEvents();

    void setupSecondaryBuffer();

#elif AUI_PLATFORM_ANDROID
    _<ASampleCommitter> mCommitter;
#endif


    void playImpl();

    void pauseImpl();

    void stopImpl();

};
