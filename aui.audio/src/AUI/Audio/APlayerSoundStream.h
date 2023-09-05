#pragma once

#include <utility>

#include "AUI/Audio/ISoundInputStream.h"

class AAudioPlayer;

/**
 * @brief Outputs modified sound data based on AAudioPlayer state
 */
class APlayerSoundStream : public ISoundInputStream {
public:
    explicit APlayerSoundStream(_weak<AAudioPlayer> player);

    size_t read(char* dst, size_t size) override;

    AAudioFormat info() override;

    void rewind() override;

private:
    _weak<AAudioPlayer> mPlayer;

    template<ASampleFormat format>
    void processSample(std::byte* src, std::byte* &dst, aui::float_within_0_1 volumeLevel);

    static constexpr size_t BUFFER_SIZE = 0x3000;
};
