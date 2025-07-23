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
    if (!mSource) return;
    mInputFormat = mSource->info();
    mOutputFormat.sampleRate = requestedSampleRate;
    mOutputFormat.channelCount = mInputFormat.channelCount;
    mOutputFormat.sampleFormat = outputSampleFormat();

    if (mInputFormat.sampleRate != mOutputFormat.sampleRate) {
        double ratio = static_cast<double>(mInputFormat.sampleRate) / static_cast<double>(requestedSampleRate);
        uint32_t request_frames = 512 * static_cast<uint32_t>(std::ceil(ratio));

        for (size_t i = 0; i < mOutputFormat.channelCount; ++i) {
            media::SincResampler::ReadCB read_cb{[this, i](int frames, float* destination) {
                readCallback(i, frames, destination);
            }};

            auto resampler = std::make_unique<media::SincResampler>(ratio, request_frames, read_cb);
            mChannels.push_back(std::move(resampler));
        }

        mInterleavedInputBuffer.reserve(request_frames * mInputFormat.channelCount * 2);
        mChannelBuffers.resize(mOutputFormat.channelCount);
        for (auto& buffer : mChannelBuffers) {
            buffer.reserve(request_frames * 2);
        }
    }
}

AAudioResampler::~AAudioResampler() {
}

size_t AAudioResampler::read(std::span<std::byte> dst) {
    if (!mSource) return 0;

    size_t samples_requested = dst.size() / aui::audio::bytesPerSample(outputSampleFormat());
    size_t frames_requested = samples_requested / mOutputFormat.channelCount;

    if (mChannels.empty()) {
        size_t bytes_to_read = dst.size();

        std::vector<char> read_buffer(bytes_to_read);
        size_t bytes_read = mSource->read(read_buffer.data(), read_buffer.size());

        if (bytes_read == 0) {
            return 0;
        }

        size_t samples_read = bytes_read / aui::audio::bytesPerSample(mInputFormat.sampleFormat);

        if (mInputFormat.sampleFormat != ASampleFormat::F32) {
            std::vector<char> float_buffer(samples_read * sizeof(float));
            aui::audio::convertSampleFormat(mInputFormat.sampleFormat, ASampleFormat::F32,
                                          read_buffer.data(), float_buffer.data(), samples_read);
            read_buffer = std::move(float_buffer);
        }

        mGainFilter.process(reinterpret_cast<float*>(read_buffer.data()), samples_read);

        if (mOutputFormat.sampleFormat != ASampleFormat::F32) {
            aui::audio::convertSampleFormat(ASampleFormat::F32, mOutputFormat.sampleFormat,
                                          read_buffer.data(), reinterpret_cast<char*>(dst.data()), samples_read);
        } else {
            std::memcpy(dst.data(), read_buffer.data(), bytes_read);
        }

        return bytes_read;
    } else {
        std::vector<float> output_buffer(samples_requested);

        for (size_t i = 0; i < mOutputFormat.channelCount; ++i) {
            mChannels[i]->Resample(frames_requested, output_buffer.data() + i * frames_requested);
        }

        std::vector<float> interleaved_output(samples_requested);
        for (size_t frame = 0; frame < frames_requested; ++frame) {
            for (size_t channel = 0; channel < mOutputFormat.channelCount; ++channel) {
                interleaved_output[frame * mOutputFormat.channelCount + channel] =
                    output_buffer[channel * frames_requested + frame];
            }
        }

        mGainFilter.process(interleaved_output.data(), samples_requested);

        if (mOutputFormat.sampleFormat != ASampleFormat::F32) {
            aui::audio::convertSampleFormat(ASampleFormat::F32, mOutputFormat.sampleFormat,
                                          reinterpret_cast<char*>(interleaved_output.data()),
                                          reinterpret_cast<char*>(dst.data()), samples_requested);
        } else {
            std::memcpy(dst.data(), interleaved_output.data(), samples_requested * sizeof(float));
        }

        return dst.size();
    }
}

ASampleFormat AAudioResampler::outputSampleFormat() {
    return aui::audio::platform::requested_sample_format;
}

void AAudioResampler::setVolume(aui::audio::VolumeLevel volume) noexcept {
    mGainFilter.setVolume(volume);
}

void AAudioResampler::readCallback(int channel, int frames, float* destination) {
    size_t samples_to_read = frames * mInputFormat.channelCount;
    size_t bytes_to_read = samples_to_read * aui::audio::bytesPerSample(mInputFormat.sampleFormat);

    if (mInterleavedInputBuffer.size() < bytes_to_read) {
        mInterleavedInputBuffer.resize(bytes_to_read);
    }

    size_t bytes_read = mSource->read(mInterleavedInputBuffer.data(), bytes_to_read);
    size_t samples_read = bytes_read / aui::audio::bytesPerSample(mInputFormat.sampleFormat);
    size_t frames_read = samples_read / mInputFormat.channelCount;

    std::vector<float> float_buffer;
    float* source_data;

    if (mInputFormat.sampleFormat != ASampleFormat::F32) {
        float_buffer.resize(samples_read);
        aui::audio::convertSampleFormat(mInputFormat.sampleFormat, ASampleFormat::F32,
                                      reinterpret_cast<char*>(mInterleavedInputBuffer.data()),
                                      reinterpret_cast<char*>(float_buffer.data()), samples_read);
        source_data = float_buffer.data();
    } else {
        source_data = reinterpret_cast<float*>(mInterleavedInputBuffer.data());
    }

    for (size_t frame = 0; frame < frames_read; ++frame) {
        destination[frame] = source_data[frame * mInputFormat.channelCount + channel];
    }

    if (frames_read < static_cast<size_t>(frames)) {
        std::memset(destination + frames_read, 0, (frames - frames_read) * sizeof(float));
    }
}
