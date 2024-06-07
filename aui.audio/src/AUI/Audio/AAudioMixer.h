#pragma once

#include <list>
#include <span>

#include <AUI/Common/AObject.h>

class API_AUI_AUDIO IAudioPlayer;

/**
 * @brief Manages audio players and mixes them into one sound stream.
 * @ingroup audio
 */
class API_AUI_AUDIO AAudioMixer {
public:
    /**
     * @brief Add new sound source for mixing
     * @param s New sound source
     */
    void addSoundSource(_<IAudioPlayer> s) noexcept;

    /**
     * @brief Remove sound source if it is represented in list of added sound sources, otherwise do nothing
     * @param s Sound source to remove
     */
    void removeSoundSource(const _<IAudioPlayer>& s);

    /**
     * @brief Write mixed audio data into buffer.
     * @param dst Buffer for writing into
     * @param size Size of mixed audio data for writing
     * @return Number of bytes written
     */
    std::size_t readSoundData(std::span<std::byte> destination);

private:
    AMutex mMutex;
    std::list<_<IAudioPlayer>> mPlayers;
};

