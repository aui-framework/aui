#pragma once

#include <cstddef>
#include <span>

#include <AUI/Common/AObject.h>
#include <AUI/Audio/Mixer/APlaybackConfig.h>

/**
 * @brief Base interface for sound sources that can be used as sound source for AAudioMixer.
 * @ingroup audio
 */
class API_AUI_AUDIO ISoundSource {
public:
    virtual ~ISoundSource() = default;

    /**
     * @brief Read sound data to the buffer
     * @param destination destination buffer
     * @return bytes written
     */
    virtual size_t readSoundData(std::span<std::byte> destination) = 0;

    /**
     * @brief Rewind the sound source to the beginning.
     * @return true if operation succeeded.
     */
    virtual bool requestRewind() = 0;
    virtual APlaybackConfig getConfig() = 0;
};
