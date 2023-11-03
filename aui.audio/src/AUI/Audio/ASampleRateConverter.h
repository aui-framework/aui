#pragma once

#include <cstddef>
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Util/APimpl.h"

typedef struct soxr * soxr_t;

class ASampleRateConverter {
public:
    explicit ASampleRateConverter(size_t requestedSampleRate, _<ISoundInputStream> stream);
    size_t convert(size_t bytesToProcess, std::span<std::byte> dst);
    ASampleFormat outputSampleFormat() const;
    ~ASampleRateConverter();

private:
    struct SoxrProcessResult {
        size_t bytesInUsed;
        size_t bytesOut;
    };

    SoxrProcessResult soxrProcess(std::span<std::byte> src, std::span<std::byte> dst);

    static constexpr size_t BUFFER_SIZE = 0x3000;
    static constexpr size_t INT32_BUFFER_SIZE = (BUFFER_SIZE / 3);

    _<ISoundInputStream> mSource;
    AAudioFormat mInputFormat;
    soxr_t mContext = nullptr;
};
