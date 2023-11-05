#include "ASampleRateConverter.h"
#include "AUI/Logging/ALogger.h"
#include <soxr.h>

namespace {
    size_t bytesToSamplesPerChannel(size_t bytes, AAudioFormat format) {
        return (bytes / aui::audio::bytesPerSample(format.sampleFormat)) / size_t(format.channelCount);
    }

    size_t samplesPerChannelToBytes(size_t spc, AAudioFormat format) {
        return spc * aui::audio::bytesPerSample(format.sampleFormat) * size_t(format.channelCount);
    }
}

ASampleRateConverter::ASampleRateConverter(size_t requestedSampleRate, _<ISoundInputStream> source) : mSource(std::move(source)) {
    mInputFormat = mSource->info();
    mOutputFormat.sampleRate = requestedSampleRate;
    mOutputFormat.channelCount = mInputFormat.channelCount;
    mOutputFormat.sampleFormat = outputSampleFormat();

    soxr_io_spec_t spec;
    switch (mInputFormat.sampleFormat) {
        case ASampleFormat::I16:
            spec.itype = SOXR_INT16_I;
            break;
        case ASampleFormat::I24:
        case ASampleFormat::I32:
            spec.itype = SOXR_INT32_I;
            break;
    }
    spec.otype = aui::platform::current::is_mobile() ? SOXR_INT16_I : SOXR_INT32_I;
    spec.scale = 1;
    spec.e = nullptr;
    spec.flags = 0;
    mContext = soxr_create(double(mInputFormat.sampleRate), double(requestedSampleRate), unsigned(mInputFormat.channelCount),
                           nullptr, &spec, nullptr, nullptr);
    if (!mContext) {
        throw AException("(soxr) Failed to initialize sample rate converter context");
    }
}

size_t ASampleRateConverter::convert(std::span<std::byte> dst) {
    size_t initialDestSize = dst.size();
    while (!dst.empty()) {
        if (mSourceBuffer.size() == 0) {
            if (mInputFormat.sampleFormat == ASampleFormat::I24) {
                mSourceBuffer.write(mSource, BUFFER_SIZE / 4 * 3);
                mSourceBuffer.convertFormatToInt32();
            }
            else {
                mSourceBuffer.write(mSource);
            }
        }

        auto result = soxrProcess(mSourceBuffer.span(), dst);
        if (result.bytesOut == 0 && result.bytesInUsed == 0) {
            break;
        }
        mSourceBuffer.begin += result.bytesInUsed;
        dst = dst.subspan(result.bytesOut);
    }

    return initialDestSize - dst.size();
}

ASampleRateConverter::~ASampleRateConverter() {
    soxr_delete(mContext);
}

ASampleRateConverter::SoxrProcessResult ASampleRateConverter::soxrProcess(std::span<std::byte> src, std::span<std::byte> dst) {
    size_t samplesInUsed;
    size_t samplesOut;
    assert(src.size() % aui::audio::bytesPerSample(mInputFormat.sampleFormat) == 0);
    auto error = soxr_process(mContext,
                              src.data(), bytesToSamplesPerChannel(src.size(), mInputFormat), &samplesInUsed,
                              dst.data(), bytesToSamplesPerChannel(dst.size(), mOutputFormat), &samplesOut);

    if (error) {
        throw AException("(soxr) error occured during resampling: {}"_format(error));
    }

    return {
        .bytesInUsed = samplesPerChannelToBytes(samplesInUsed, mInputFormat),
        .bytesOut = samplesPerChannelToBytes(samplesOut, mOutputFormat)
    };
}

size_t ASampleRateConverter::AuxBuffer::read(char *dst, size_t size) {
    size_t bytesToCopy = std::min(size, end - begin);
    std::memcpy(dst, buffer, bytesToCopy);
    begin += bytesToCopy;
    return bytesToCopy;
}

void ASampleRateConverter::AuxBuffer::write(const _<IInputStream> &source, size_t bytesToRead) {
    begin = 0;
    end = source->read(buffer, std::min(BUFFER_SIZE, bytesToRead));
}

size_t ASampleRateConverter::AuxBuffer::size() const {
    return end - begin;
}

std::span<std::byte> ASampleRateConverter::AuxBuffer::span() {
    assert(0 <= begin && begin <= BUFFER_SIZE);
    assert(0 <= end && end <= BUFFER_SIZE);
    assert(begin <= end);
    return {reinterpret_cast<std::byte*>(buffer) + begin, end - begin};
}

void ASampleRateConverter::AuxBuffer::convertFormatToInt32() {
    assert(("convertFormatToInt32 must be called immedeatly after writing data to buffer", begin == 0));
    assert(("wrong amount of data has been read or converting is not needed", size() % 3 == 0));
    assert(("too much data has been read", 4 * size() / 3 <= BUFFER_SIZE));
    if (size() == 0) {
        return;
    }

    size_t sampleCount = size() / 3;
    end = 4 * sampleCount;
    uint32_t* outPtr = reinterpret_cast<uint32_t*>(buffer) + (sampleCount - 1);
    char* inPtr = buffer + 3 * sampleCount;
    while (inPtr != buffer) {
        inPtr -= 3;
        --outPtr;
        *outPtr = (*reinterpret_cast<uint32_t*>(inPtr)) << 8;
    }
}
