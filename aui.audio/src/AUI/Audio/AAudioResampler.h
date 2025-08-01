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

    void setVolume(aui::audio::VolumeLevel volume) noexcept;

    [[nodiscard]]
    const _<ISoundInputStream>& source() const { return mSource; }

private:
    void readCallback(size_t channel, int frames, float* destination);
    void readFrame();
    static void mixChannels(const float* input, size_t inputFrames, size_t inChannels,
                          float* output, size_t outChannels);

    AGainFilter mGainFilter;
    size_t mRequestFrames;
    ASmallVector<std::unique_ptr<media::SincResampler>, 2> mChannels;
    std::vector<std::vector<float>> mChannelBuffers;
    std::vector<char> mReadBuffer;
    std::vector<char> mConvertedBuffer;
    std::vector<float> mResamplingBuffer;
    std::vector<char> mMixingBuffer;
    _<ISoundInputStream> mSource;
    AAudioFormat mInputFormat;
    AAudioFormat mOutputFormat;
};
