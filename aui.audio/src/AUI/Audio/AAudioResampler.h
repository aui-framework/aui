#pragma once

#include <cstddef>
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
    size_t process(std::span<std::byte> dst);

    /**
     * @return Sample format that will be used in process()
     */
    ASampleFormat outputSampleFormat();

    [[nodiscard]]
    const _<ISoundInputStream>& source() const { return mSource; }

private:
    void readCallback(int frames, float* destination);

    ASmallVector<std::unique_ptr<media::SincResampler>, 2> mChannels;
    _<ISoundInputStream> mSource;
    AAudioFormat mInputFormat;
    AAudioFormat mOutputFormat;
};
