#pragma once

#include "AUI/Audio/Sound/ISoundStream.h"
#include <list>
#include <mutex>

class ISoundSource;

class AAudioMixer {
private:
    std::mutex mMutex;
    std::list<_<ISoundSource>> mSoundSources;

public:
    void addSoundSource(_<ISoundSource> s);
    void removeSoundSource(const _<ISoundSource>& s);
    size_t requestSoundData(char* dst, size_t size);
};

