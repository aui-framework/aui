#pragma once

#include <cstddef>
#include "AUI/Audio/AGainFilter.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Util/APimpl.h"
#include "AUI/Traits/platform.h"
#include "AUI/Common/ASmallVector.h"

namespace media {
class SincResampler;
}

class AAudioResampler {
public:
    AAudioResampler(size_t requestedSampleRate, _<ISoundInputStream> stream);

    ~AAudioResampler();

    /**
     * @brief Performs sample rate converting
     * @param dst destination buffer in which converted data will be written
     * @return Size of outputted data in bytes
     */
    size_t read(std::span<std::byte> dst);

    /**
     * @return Sample format that will be used in process()
     */
    ASampleFormat outputSampleFormat();

    void setVolume(aui::audio::VolumeLevel volume) noexcept;

    [[nodiscard]]
    const _<ISoundInputStream>& source() const { return mSource; }

private:
    void readCallback(int channel, int frames, float* destination);

    AGainFilter mGainFilter;
    ASmallVector<std::unique_ptr<media::SincResampler>, 2> mChannels;
    std::vector<char> mInterleavedInputBuffer;
    std::vector<std::vector<float>> mChannelBuffers;
    _<ISoundInputStream> mSource;
    AAudioFormat mInputFormat;
    AAudioFormat mOutputFormat;
};
