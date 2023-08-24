#pragma once

#include <cstddef>
#include "AUI/Audio/Mixer/PlaybackConfig.h"

/**
 * @brief Base interface for sound sources that can be used as sound source for AAudioMixer
 */
class ISoundSource {
public:
    virtual size_t requestSoundData(char* dst, size_t size) = 0;
    virtual bool requestRewind() = 0;
    virtual PlaybackConfig getConfig() = 0;
};
