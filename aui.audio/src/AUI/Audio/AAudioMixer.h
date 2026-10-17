#pragma once

#include "AUI/Common/AStaticVector.h"

#include <list>
#include <span>

#include <AUI/Common/AObject.h>

class API_AUI_AUDIO IAudioPlayer;

/**
 * @brief Manages audio players and mixes them into one sound stream.
 * @ingroup audio
 * @details
 * <!-- aui:experimental -->
 *
 * The mixer sums the output of all [IAudioPlayer]s which are currently playing and clamps the result. It is driven by
 * the platform audio backend: `IAudioPlayer::play` adds the player to the mixer, `pause` and `stop` remove it.
 * Applications normally don't use the mixer directly.
 *
 * Players that reach the end of their stream (and are not looping) or throw while being read are removed and notified
 * with `IAudioPlayer::finished`.
 */
class API_AUI_AUDIO AAudioMixer {
public:
    static constexpr auto MAX_PLAYER_COUNT = 100;

    /**
     * @brief Add new sound source for mixing
     * @param s New sound source
     */
    void addSoundSource(aui::non_null<_<IAudioPlayer>> s);

    /**
     * @brief Remove sound source if it is represented in list of added sound sources, otherwise do nothing
     * @param s Sound source to remove
     */
    void removeSoundSource(aui::non_null<_<IAudioPlayer>> s);

    /**
     * @brief Write mixed audio data into buffer.
     * @param destination Pre-allocated buffer to write into
     * @return Number of bytes written
     */
    std::size_t readSoundData(std::span<std::byte> destination);

    ~AAudioMixer();

private:
    ASpinlockMutex mConcurrentAccessCheck;
    AStaticVector<_<IAudioPlayer>, MAX_PLAYER_COUNT> mPlayers;
    std::vector<float> mMixBuffer;
    std::vector<float> mReadBuffer;
};

