#include "AAudioResampler.h"
#include "Platform/RequestedAudioFormat.h"
#include "SincResampler.h"

namespace {
    size_t bytesToSamplesPerChannel(size_t bytes, AAudioFormat format) {
        return (bytes / aui::audio::bytesPerSample(format.sampleFormat)) / size_t(format.channelCount);
    }

    size_t samplesPerChannelToBytes(size_t spc, AAudioFormat format) {
        return spc * aui::audio::bytesPerSample(format.sampleFormat) * size_t(format.channelCount);
    }
}

AAudioResampler::AAudioResampler(size_t requestedSampleRate, _<ISoundInputStream> source) : mSource(std::move(source)) {
    mInputFormat = mSource->info();
    mOutputFormat.sampleRate = requestedSampleRate;
    mOutputFormat.channelCount = mInputFormat.channelCount;
    mOutputFormat.sampleFormat = outputSampleFormat();

    if (mInputFormat.sampleRate != mOutputFormat.sampleRate) {
        double ratio = static_cast<double>(mInputFormat.sampleRate) / static_cast<double>(requestedSampleRate);
        uint32_t request_frames = 512 * static_cast<uint32_t>(std::ceil(ratio));
        media::SincResampler::ReadCB read_cb{[this](int frames, float* destination) {
            readCallback(frames, destination);
        }};

        for (size_t i = 0; i < mOutputFormat.channelCount; ++i) {
            auto resampler = std::make_unique<media::SincResampler>(ratio, request_frames, read_cb);
            mChannels.push_back(std::move(resampler));
        }
    }
}

AAudioResampler::~AAudioResampler() {
}

size_t AAudioResampler::process(std::span<std::byte> dst) {
    size_t frames_requested = dst.size() / sizeof(float);

    if (mChannels.empty()) {

    } else {
        int to_read = mChannels.first()->GetMaxInputFramesRequested(frames_requested);
        std::vector<char> read_buffer(to_read * aui::audio::bytesPerSample(mInputFormat.sampleFormat));
        read_buffer.resize(mSource->read(read_buffer.data(), read_buffer.size()));
    }

    return dst.size();
}

ASampleFormat AAudioResampler::outputSampleFormat() {
    return aui::audio::platform::requested_sample_format;
}

void AAudioResampler::readCallback(int frames, float* destination) {
    // TODO: single call and then deinterleave and then provide to destination
}
