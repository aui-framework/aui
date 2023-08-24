#pragma once

#include <cstddef>
#include "AUI/Audio/Mixer/PlaybackConfig.h"

class ISoundSource {
public:
    virtual size_t requestSoundData(char* dst, size_t size) = 0;
    virtual bool requestRewind() = 0;
    virtual PlaybackConfig getConfig() = 0;
};
