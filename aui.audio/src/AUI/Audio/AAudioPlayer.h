#pragma once

#if AUI_PLATFORM_WIN
#include <Windows.h>
#endif

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Audio/AAudioMixer.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Util/APimpl.h"


/**
 * @brief Interface for audio playback.
 * @ingroup audio
 */
class API_AUI_AUDIO AAudioPlayer: public AObject {
public:
    AAudioPlayer();
    ~AAudioPlayer() override;

    explicit AAudioPlayer(_<ISoundInputStream> stream);

    /**
     * @brief Playback status depends on last called function among play(), pause(), stop().
     */
    enum class PlaybackStatus : int8_t {
        PLAYING,
        PAUSED,
        STOPPED
    };

    /**
     * @brief Starts audio playback, if playback was previously paused, it will continue from where it was paused.
     */
    void play() {
        if (mPlaybackStatus != PlaybackStatus::PLAYING) {
            playImpl();
            mPlaybackStatus = PlaybackStatus::PLAYING;
        }
    }

    /**
     * @brief Pauses audio playback keeping playback progress.
     */
    void pause() {
        if (mPlaybackStatus == PlaybackStatus::PLAYING) {
            pauseImpl();
            mPlaybackStatus = PlaybackStatus::PAUSED;
        }
    }

    /**
     * @brief Pauses audio playback without keeping playback progress.
     */
    void stop() {
        if (mPlaybackStatus != PlaybackStatus::STOPPED) {
            stopImpl();
            mPlaybackStatus = PlaybackStatus::STOPPED;
        }
    }

    /**
     * @return Current playback status.
     */
    PlaybackStatus getStatus() const noexcept {
        return mPlaybackStatus;
    }

    /**
     * @brief Sets new source for playback.
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
     * @brief Get source for playback.
     */
    [[nodiscard]]
    const _<ISoundInputStream>& source() const noexcept {
        return mSource;
    }

    /**
     * @brief Get resampled stream for playback.
     */
    [[nodiscard]]
    const _<ISoundInputStream>& resampledStream() const noexcept {
        return mResampler != nullptr ? mResampler : mSource;
    }

    /**
     * @brief Set loop flag, is loop flag is true then audio playback wouldn't be stopped after it ends and
     * sound stream would be rewind.
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
     * @brief Set level of volume.
     * @param volume Float number from 0 to 1 inclusively
     */
    void setVolume(aui::float_within_0_1 volume) {
        mVolume = volume;
        onVolumeSet();
    }

    /**
     * @return Current volume level.
     */
    [[nodiscard]]
    aui::float_within_0_1 volume() const noexcept {
        return mVolume;
    }

signals:
    /**
     * @brief On playback finished.
     */
    emits<> finished;

private:
    _<ISoundInputStream> mSource;
    PlaybackStatus mPlaybackStatus = PlaybackStatus::STOPPED;
    bool mLoop = false;
    aui::float_within_0_1 mVolume = 1.f;
    _<ISoundInputStream> mResampler;

#if AUI_PLATFORM_WIN
    static constexpr int BUFFER_DURATION_SEC = 2;
    static_assert(BUFFER_DURATION_SEC >= 2 && "Buffer duration assumes to be greater than 1");

    HANDLE mEvents[BUFFER_DURATION_SEC + 1];
    HANDLE mThread;
    bool mThreadIsActive = false;

    struct Private;
    aui::fast_pimpl<Private, (sizeof(void*) + sizeof(long)) * (2 + BUFFER_DURATION_SEC), alignof(void*)> mPrivate;

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
#endif


    void playImpl();

    void pauseImpl();

    void stopImpl();

    void onSourceSet();

    void onVolumeSet();
};
