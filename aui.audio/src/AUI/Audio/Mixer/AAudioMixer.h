#pragma once

#include "AUI/Audio/Sound/ISoundStream.h"
#include "AUI/Audio/Mixer/details/ISoundSource.h"
#include <list>
#include <mutex>

class AAudioMixer: public ISoundSource {
private:
    std::mutex mMutex;
    std::list<_<ISoundSource>> mSoundSources;

public:
    void addSoundSource(_<ISoundSource> s);
    void removeSoundSource(const _<ISoundSource>& s);

    size_t requestSoundData(char* dst, size_t size) override;
};

