#pragma once

#include <cstddef>
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Util/APimpl.h"
#include "AUI/Traits/platform.h"

typedef struct soxr * soxr_t;

class ASampleRateConverter {
public:
    ASampleRateConverter(size_t requestedSampleRate, _<ISoundInputStream> stream);

    /**
     * @brief Performs sample rate converting
     * @param dst destination buffer in which converted data will be written
     * @return Size of outputted data in bytes
     */
    size_t convert(std::span<std::byte> dst);

    /**
     * @return Sample format that will be used in convert()
     */
    static constexpr ASampleFormat outputSampleFormat() {
        return aui::platform::current::is_mobile() ? ASampleFormat::I16 : ASampleFormat::I32;
    }

    ~ASampleRateConverter();

private:
    struct SoxrProcessResult {
        size_t bytesInUsed;
        size_t bytesOut;
    };

    SoxrProcessResult soxrProcess(std::span<std::byte> src, std::span<std::byte> dst);

    static constexpr size_t BUFFER_SIZE = 0x3000;
    static constexpr size_t INT32_BUFFER_SIZE = (BUFFER_SIZE / 3);

    struct AuxBuffer {
        size_t read(char* dst, size_t size);

        void write(const _<IInputStream>& source, size_t bytesToRead = BUFFER_SIZE);

        [[nodiscard]]
        size_t size() const;

        std::span<std::byte> span();

        /**
         * @brief Converts in-place data in buffer from 24 bits per sample to 32 bits per sample format.
         * @note 32 bits per smaple format needed by libsoxr.
         */
        void convertFormatToInt32();

        char buffer[BUFFER_SIZE];
        size_t begin = 0;
        size_t end = 0;
    };



    soxr_t mContext = nullptr;
    _<ISoundInputStream> mSource;
    AAudioFormat mInputFormat;
    AAudioFormat mOutputFormat;
    AuxBuffer mSourceBuffer;
};
