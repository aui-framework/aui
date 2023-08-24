#pragma once

#include "AUI/Audio/Mixer/ISoundSource.h"
#include "AUI/Audio/Sound/ISoundStream.h"

class AAudioPlayer;

/**
 * @brief Audio data committer that can be used as sound source for AAudioMixer
 */
class ASampleCommitter : public ISoundSource {
public:
    explicit ASampleCommitter(_<ISoundStream> stream, PlaybackConfig config);
    size_t requestSoundData(char* dst, size_t size) override;
    bool requestRewind() override;
    PlaybackConfig getConfig() override;

private:
    _<ISoundStream> mSoundStream;
    AAudioFormat mFormat;
    PlaybackConfig mConfig;
};