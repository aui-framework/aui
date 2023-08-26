#pragma once

#include "AUI/Audio/Mixer/ISoundSource.h"
#include "AUI/Audio/Sound/ISoundInputStream.h"

class AAudioPlayer;

/**
 * @brief Audio data committer that can be used as sound source for AAudioMixer
 */
class ASampleCommitter : public ISoundSource {
public:
    explicit ASampleCommitter(_<ISoundInputStream> stream, PlaybackConfig config);
    size_t readSoundData(std::span<std::byte> destination) override;
    bool requestRewind() override;
    PlaybackConfig getConfig() override;

private:
    _<ISoundInputStream> mSoundStream;
    AAudioFormat mFormat;
    PlaybackConfig mConfig;
};