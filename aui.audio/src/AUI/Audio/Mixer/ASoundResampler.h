#pragma once

#include "AUI/Audio/Mixer/ISoundSource.h"
#include "AUI/Audio/Sound/ISoundInputStream.h"

class AAudioPlayer;

/**
 * @brief Implements audio mixing and resampling.
 * @ingroup audio
 */
class ASoundResampler : public ISoundSource {
public:
    explicit ASoundResampler(_<ISoundInputStream> stream, APlaybackConfig config);
    size_t readSoundData(std::span<std::byte> destination) override;
    bool requestRewind() override;
    APlaybackConfig getConfig() override;

private:
    _<ISoundInputStream> mSoundStream;
    AAudioFormat mFormat;
    APlaybackConfig mConfig;
};