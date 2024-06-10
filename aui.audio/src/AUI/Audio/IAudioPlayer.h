#pragma once

#include <AUI/Common/AObject.h>
#include <AUI/Common/ASignal.h>
#include <AUI/Url/AUrl.h>
#include <AUI/Audio/ASoundResampler.h>
#include <AUI/Audio/VolumeLevel.h>

/**
 * @brief Interface for audio playback.
 * @ingroup audio
 */
class API_AUI_AUDIO IAudioPlayer: public AObject {
public:
    explicit IAudioPlayer(AUrl url);

    static _<IAudioPlayer> fromUrl(AUrl url);
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
    void play();

    /**
     * @brief Pauses audio playback keeping playback progress.
     */
    void pause();

    /**
     * @brief Pauses audio playback without keeping playback progress.
     */
    void stop();

    /**
     * @return Current playback status.
     */
    PlaybackStatus playbackStatus() const noexcept {
        return mPlaybackStatus;
    }

    /**
     * @brief Get resampled stream for playback.
     */
    [[nodiscard]]
    const _<ASoundResampler>& resampledStream() const noexcept {
        return mResampledStream;
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

    /**
     * @brief Set level of volume.
     * @param volume Integer number from 0 to 256 inclusively, works linear
     */
    void setVolume(aui::audio::VolumeLevel volume) {
        mVolume = volume;
        AUI_NULLSAFE(mResampledStream)->setVolume(volume);
        onVolumeSet();
    }

    /**
     * @return Current volume level.
     */
    [[nodiscard]]
    aui::audio::VolumeLevel volume() const noexcept {
        return mVolume;
    }

    /**
     * @brief Called By AAudioMixer when the playback is finished.
     * @details
     * See IAudioPlayer::finished for listening for this event.
     */
    void onFinished() {
        emit finished;
    }

    const AUrl& url() const noexcept {
        return mUrl;
    }

    void rewind() {
        release();
        initialize();
    }

signals:
    /**
     * @brief On playback finished.
     */
    emits<> finished;

protected:
    bool isInitialized() const noexcept {
        return mResampledStream != nullptr;
    }

    void initialize();

    void initializeIfNeeded() {
        if (!isInitialized()) {
            initialize();
        }
    }

    void release();

private:
    aui::audio::VolumeLevel mVolume = aui::audio::VolumeLevel::MAX;
    AUrl mUrl;
    _<ISoundInputStream> mSourceStream;
    _<ASoundResampler> mResampledStream;
    PlaybackStatus mPlaybackStatus = PlaybackStatus::STOPPED;
    bool mLoop = false;

    virtual void playImpl() = 0;
    virtual void pauseImpl() = 0;
    virtual void stopImpl() = 0;

    virtual void onVolumeSet() { }
    virtual void onLoopSet() { }
};
