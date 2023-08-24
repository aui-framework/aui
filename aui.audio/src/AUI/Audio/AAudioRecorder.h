#pragma once

#include "AUI/Audio/Sound/RawSoundStream.h"

class AAudioRecorder {
public:
    enum class RecordingStatus : int8_t {
        RECORDING,
        STOPPED
    };

    void start() {
        startImpl();
        mStatus = RecordingStatus::RECORDING;
    }

    _<RawSoundStream> stop() {
        auto res = stopImpl();
        mStatus = RecordingStatus::STOPPED;
        return res;
    }

    RecordingStatus getStatus() const noexcept {
        return mStatus;
    }

private:
    RecordingStatus mStatus = RecordingStatus::STOPPED;

    void startImpl();

    _<RawSoundStream> stopImpl();
};
