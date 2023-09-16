#pragma once

#include <utility>

#include "AUI/Audio/ISoundInputStream.h"

class AAudioPlayer;

/**
 * @brief Outputs modified sound data based on AAudioPlayer state
 */
class APlayerSoundStream : public ISoundInputStream {
public:
    explicit APlayerSoundStream(AAudioPlayer* player);

    size_t read(char* dst, size_t size) override;

    AAudioFormat info() override;

    void rewind() override;

private:
    AAudioPlayer* mPlayer;


    static constexpr size_t BUFFER_SIZE = 0x3000;
    static constexpr int32_t MAX_VOLUME_LEVEL = 256;
    using VolumeLevel = aui::ranged_number<int32_t, 0, 256>;

    template<ASampleFormat format>
    void processSample(std::byte* src, std::byte* &dst, VolumeLevel volumeLevel);
};
