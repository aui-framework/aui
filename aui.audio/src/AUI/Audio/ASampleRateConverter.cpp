#include "ASampleRateConverter.h"
#include <soxr.h>

namespace {
    void int24ToInt32(uint32_t* dst, size_t size, const char* src) {
        assert(size % 3 == 0);
        auto ptr = reinterpret_cast<const uint32_t*>(src);
        while (size >= 12) {
            dst[0] = ptr[0] << 8;
            dst[1] = ((ptr[0] >> 16) | (ptr[1] << 16)) & 0xffffff00;
            dst[2] = ((ptr[1] >> 8) | (ptr[2] << 24)) & 0xffffff00;
            dst[3] = ptr[2] & 0xffffff00;
            dst += 4;
            ptr += 3;
            size -= 12;
        }

        for (int i = 0; i < size; i += 3) {
            std::memcpy(reinterpret_cast<char*>(dst) + 4 * (i / 3) + 1, reinterpret_cast<const char*>(ptr) + i, 3);
            size -= 3;
        }
    }

    size_t bytesToSamplesPerChannel(size_t bytes, AAudioFormat format) {
        return (bytes / aui::audio::bytesPerSample(format.sampleFormat)) / size_t(format.channelCount);
    }

    size_t samplesPerChannelToBytes(size_t spc, AAudioFormat format) {
        return spc * aui::audio::bytesPerSample(format.sampleFormat) * size_t(format.channelCount);
    }
}

ASampleRateConverter::ASampleRateConverter(size_t requestedSampleRate, _<ISoundInputStream> source) : mSource(std::move(source)) {
    mInputFormat = mSource->info();

    if (mInputFormat.sampleRate != requestedSampleRate) {
        soxr_io_spec_t spec;
        switch (mInputFormat.sampleFormat) {
            case ASampleFormat::I16:
                spec.itype = SOXR_INT16_I;
                spec.otype = SOXR_INT16_I;
                break;
            case ASampleFormat::I24:
            case ASampleFormat::I32:
                spec.itype = SOXR_INT32_I;
                spec.otype = SOXR_INT32_I;
                break;
        }
        spec.scale = 1;
        spec.e = nullptr;
        spec.flags = 0;
        mContext = soxr_create(double(mInputFormat.sampleRate), double(requestedSampleRate), unsigned(mInputFormat.channelCount),
                               nullptr, &spec, nullptr, nullptr);
        if (!mContext) {
            throw AException("Failed to initialize sample rate converter context");
        }
    }
}

size_t ASampleRateConverter::convert(size_t bytesToProcess, std::span<std::byte> dst) {
    if (!mContext) {
        return mSource->read(reinterpret_cast<char*>(dst.data()), bytesToProcess);
    }

    char buffer[BUFFER_SIZE];
    size_t result = 0;
    while (bytesToProcess > 0) {
        size_t bytesRead = mSource->read(buffer, glm::min(bytesToProcess, sizeof(buffer)));
        bytesToProcess -= bytesRead;
        SoxrProcessResult processResult{};
        if (mInputFormat.sampleFormat == ASampleFormat::I24) {
            uint32_t auxBuffer[INT32_BUFFER_SIZE];
            int24ToInt32(auxBuffer, bytesRead, buffer);
            processResult = soxrProcess({reinterpret_cast<std::byte*>(auxBuffer), (bytesRead / 3) * 4}, dst);
        }
        else {
            processResult = soxrProcess({reinterpret_cast<std::byte*>(buffer), bytesRead}, dst);
        }

        dst = dst.subspan(processResult.bytesOut);
        result += processResult.bytesOut;
    }

    return result;
}

ASampleRateConverter::~ASampleRateConverter() {
    soxr_delete(mContext);
}

ASampleFormat ASampleRateConverter::outputSampleFormat() const {
    return mInputFormat.sampleFormat == ASampleFormat::I16 ? ASampleFormat::I16 : ASampleFormat::I32;
}

ASampleRateConverter::SoxrProcessResult ASampleRateConverter::soxrProcess(std::span<std::byte> src, std::span<std::byte> dst) {
    SoxrProcessResult result;
    assert(src.size() % aui::audio::bytesPerSample(mInputFormat.sampleFormat) == 0);
    assert(dst.size() % aui::audio::bytesPerSample(outputSampleFormat()) == 0);
    auto error = soxr_process(mContext,
                              src.data(), bytesToSamplesPerChannel(src.size(), mInputFormat), &result.bytesInUsed,
                              dst.data(), bytesToSamplesPerChannel(dst.size(), mInputFormat), &result.bytesOut);

    if (error) {
        throw AException("(soxr) error occured during resampling: {}"_format(error));
    }

    return result;
}
