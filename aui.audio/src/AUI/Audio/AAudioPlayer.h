#pragma once

#include "AUI/Audio/Sound/ISoundInputStream.h"
#include "AUI/Audio/Mixer/AAudioMixer.h"

#if AUI_PLATFORM_WIN
#include "AUI/Audio/Platform/win32/DirectSound.h"
#else
#include "AUI/Audio/Mixer/ISoundSource.h"
#endif

/**
 * @brief Interface for audio playback
 * @ingroup audio
 */
class AAudioPlayer {
public:
    AAudioPlayer() = default;

    explicit AAudioPlayer(_<ISoundInputStream> stream) {
        setSource(std::move(stream));
    }

    /**
     * @brief Playback status depends on last called function among play(), pause(), stop()
     */
    enum class PlaybackStatus : int8_t {
        PLAYING,
        PAUSED,
        STOPPED
    };

    /**
     * @brief Starts audio playback, if playback was previously paused, it will continue from where it was paused
     */
    void play() {
        if (mPlaybackStatus != PlaybackStatus::PLAYING) {
            playImpl();
            mPlaybackStatus = PlaybackStatus::PLAYING;
        }
    }

    /**
     * @brief Pauses audio playback keeping playback progress
     */
    void pause() {
        if (mPlaybackStatus == PlaybackStatus::PLAYING) {
            pauseImpl();
            mPlaybackStatus = PlaybackStatus::PAUSED;
        }
    }

    /**
     * @brief Pauses audio playback without keeping playback progress
     */
    void stop() {
        if (mPlaybackStatus != PlaybackStatus::STOPPED) {
            stopImpl();
            mPlaybackStatus = PlaybackStatus::STOPPED;
        }
    }

    /**
     * @return Current playback status
     */
    PlaybackStatus getStatus() const noexcept {
        return mPlaybackStatus;
    }

    /**
     * @brief Sets new source for playback
     * @param src
     */
    void setSource(_<ISoundInputStream> src) {
        if (mPlaybackStatus != PlaybackStatus::STOPPED) {
            stop();
        }
        mSource = std::move(src);
        onSourceSet();
    }

    /**
     * @brief Set loop flag, is loop flag is true then audio playback wouldn't be stopped after it ends and
     * sound stream would be rewind
     * @param loop New loop flag
     */
    void setLoop(bool loop) {
        mLoop = loop;
    }

    /**
     * @return Current loop flag
     */
    [[nodiscard]]
    bool loop() const noexcept {
        return mLoop;
    }

    /**
     * @brief Set level of volume
     * @param volume Float number from 0 to 1 inclusively
     */
    void setVolume(aui::float_within_0_1 volume) {
        mVolume = volume;
    }

    /**
     * @return Current volume level
     */
    [[nodiscard]]
    aui::float_within_0_1 volume() const noexcept {
        return mVolume;
    }

private:
    _<ISoundInputStream> mSource;
    PlaybackStatus mPlaybackStatus = PlaybackStatus::STOPPED;
    bool mLoop = false;
    aui::float_within_0_1 mVolume = 1.f;

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
#else
    _<ISoundSource> mResampler;
#endif


    void playImpl();

    void pauseImpl();

    void stopImpl();

    void onSourceSet();

};
