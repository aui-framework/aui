#pragma once

#include "AUI/Audio/Sound/ARawSoundStream.h"

/**
 * @brief Interface for audio recording
 */
class AAudioRecorder {
public:
    enum class RecordingStatus : int8_t {
        RECORDING,
        STOPPED
    };

    void start() {
        if (mStatus != RecordingStatus::RECORDING) {
            startImpl();
            mStatus = RecordingStatus::RECORDING;
        }
    }

    _<ARawSoundStream> stop() {
        if (mStatus != RecordingStatus::RECORDING) {
            return nullptr;
        }
        auto res = stopImpl();
        mStatus = RecordingStatus::STOPPED;
        return res;
    }

    [[nodiscard]]
    RecordingStatus getStatus() const noexcept {
        return mStatus;
    }

private:
    RecordingStatus mStatus = RecordingStatus::STOPPED;

    void startImpl();

    _<ARawSoundStream> stopImpl();
};
