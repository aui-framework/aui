#pragma once

#include "AUI/Audio/Sound/ISoundStream.h"
#include <list>
#include <mutex>

class ISoundSource;

/**
 * @brief Audio mixer, mixes several audio streams into one
 */
class AAudioMixer {
private:
    std::mutex mMutex;
    std::list<_<ISoundSource>> mSoundSources;

public:
    /**
     * @brief Add new sound source for mixing
     * @param s New sound source
     */
    void addSoundSource(_<ISoundSource> s);

    /**
     * @brief Remove sound source if it is represented in list of added sound sources, otherwise do nothing
     * @param s Sound source to remove
     */
    void removeSoundSource(const _<ISoundSource>& s);

    /**
     * @brief Write mixed audio data into buffer dst
     * @param dst Buffer for writing into
     * @param size Size of mixed audio data for writing
     * @return Number of bytes written
     */
    size_t requestSoundData(char* dst, size_t size);
};

