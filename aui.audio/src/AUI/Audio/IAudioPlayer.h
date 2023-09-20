#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>

class ISoundInputStream;
class AUrl;

/**
 * @brief Interface for audio playback.
 * @ingroup audio
 */
class API_AUI_AUDIO IAudioPlayer: public AObject {
public:

    static _<IAudioPlayer> fromUrl(const AUrl& url);

    static _<IAudioPlayer> fromSoundStream(_<ISoundInputStream>);

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
    PlaybackStatus playbackStatus() const noexcept {
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
        return mResampled != nullptr ? mResampled : mSource;
    }

    /**
     * @brief Set loop flag, is loop flag is true then audio playback wouldn't be stopped after it ends and
     * sound stream would be rewind.
     * @param loop New loop flag
     */
    void setLoop(bool loop) {
        mLoop = loop;
        onLoopSet();
    }

    /**
     * @return Current loop flag
     */
    [[nodiscard]]
    bool loop() const noexcept {
        return mLoop;
    }

    using VolumeLevel = aui::ranged_number<uint32_t, 0, 256>;

    /**
     * @brief Set level of volume.
     * @param volume Float number from 0 to 1 inclusively
     */
    void setVolume(VolumeLevel volume) {
        mVolume = volume;
        onVolumeSet();
    }

    /**
     * @return Current volume level.
     */
    [[nodiscard]]
    VolumeLevel volume() const noexcept {
        return mVolume;
    }

    void rewind() {
        stop();
        play();
    }

signals:
    /**
     * @brief On playback finished.
     */
    emits<> finished;

protected:
    _<ISoundInputStream> mResampled;

private:
    _<ISoundInputStream> mSource;
    PlaybackStatus mPlaybackStatus = PlaybackStatus::STOPPED;
    bool mLoop = false;

    /**
     * @brief Volume level, integer from 0 to 256, works linear
     */
    VolumeLevel mVolume = 256;

    virtual void playImpl() = 0;
    virtual void pauseImpl() = 0;
    virtual void stopImpl() = 0;

    virtual void onSourceSet() { }
    virtual void onVolumeSet() { }
    virtual void onLoopSet() { }
};
