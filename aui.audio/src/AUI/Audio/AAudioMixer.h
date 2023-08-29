#pragma once

#include <list>
#include <mutex>
#include <span>

#include <AUI/Common/AObject.h>

class API_AUI_AUDIO AAudioPlayer;

/**
 * @brief Audio mixer, mixes several audio streams into one.
 * @ingroup audio
 */
class API_AUI_AUDIO AAudioMixer {
private:
    std::mutex mMutex;
    std::list<_<AAudioPlayer>> mPlayers;

public:
    /**
     * @brief Add new sound source for mixing
     * @param s New sound source
     */
    void addSoundSource(_<AAudioPlayer> s);

    /**
     * @brief Remove sound source if it is represented in list of added sound sources, otherwise do nothing
     * @param s Sound source to remove
     */
    void removeSoundSource(const _<AAudioPlayer>& s);

    /**
     * @brief Write mixed audio data into buffer.
     * @param dst Buffer for writing into
     * @param size Size of mixed audio data for writing
     * @return Number of bytes written
     */
    std::size_t readSoundData(std::span<std::byte> destination);
};
