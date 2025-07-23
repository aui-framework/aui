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
        mRequestFrames = 512 * static_cast<uint32_t>(std::ceil(ratio));

        for (size_t i = 0; i < mInputFormat.channelCount; ++i) {
            media::SincResampler::ReadCB read_cb{[this, i](int frames, float* destination) {
                readCallback(i, frames, destination);
            }};

            auto resampler = std::make_unique<media::SincResampler>(ratio, mRequestFrames, read_cb);
            mChannels.push_back(std::move(resampler));
        }

        mChannelBuffers.resize(mInputFormat.channelCount);
        for (auto& buffer : mChannelBuffers) {
            buffer.reserve(mRequestFrames * 2);
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
        size_t input_frames_to_read = frames_requested;
        size_t input_samples_to_read = input_frames_to_read * mInputFormat.channelCount;
        size_t bytes_to_read = input_samples_to_read * aui::audio::bytesPerSample(mInputFormat.sampleFormat);

        std::vector<char> read_buffer(bytes_to_read);
        size_t bytes_read = mSource->read(read_buffer.data(), read_buffer.size());

        if (bytes_read == 0) {
            return 0;
        }

        size_t input_samples_read = bytes_read / aui::audio::bytesPerSample(mInputFormat.sampleFormat);
        size_t input_frames_read = input_samples_read / mInputFormat.channelCount;

        std::vector<float> float_buffer;
        float* processing_data;

        if (mInputFormat.sampleFormat != ASampleFormat::F32) {
            float_buffer.resize(input_samples_read);
            aui::audio::convertSampleFormat(mInputFormat.sampleFormat, ASampleFormat::F32,
                                          read_buffer.data(), reinterpret_cast<char*>(float_buffer.data()), input_samples_read);
            processing_data = float_buffer.data();
        } else {
            processing_data = reinterpret_cast<float*>(read_buffer.data());
        }

        mGainFilter.process(processing_data, input_samples_read);

        std::vector<float> channel_converted_buffer;
        float* output_data;
        size_t output_samples;

        if (mInputFormat.channelCount != mOutputFormat.channelCount) {
            output_samples = input_frames_read * mOutputFormat.channelCount;
            channel_converted_buffer.resize(output_samples);

            mixChannels(processing_data, input_frames_read,
                          mInputFormat.channelCount,
                          channel_converted_buffer.data(),
                          mOutputFormat.channelCount);

            output_data = channel_converted_buffer.data();
        } else {
            output_data = processing_data;
            output_samples = input_samples_read;
        }

        if (mOutputFormat.sampleFormat != ASampleFormat::F32) {
            aui::audio::convertSampleFormat(ASampleFormat::F32, mOutputFormat.sampleFormat,
                                          reinterpret_cast<char*>(output_data),
                                          reinterpret_cast<char*>(dst.data()), output_samples);
        } else {
            std::memcpy(dst.data(), output_data, output_samples * sizeof(float));
        }

        return output_samples * aui::audio::bytesPerSample(mOutputFormat.sampleFormat);
    } else {
        std::vector<float> resampled_buffer(frames_requested * mInputFormat.channelCount);
        for (size_t i = 0; i < mInputFormat.channelCount; ++i) {
            mChannels[i]->Resample(static_cast<int>(frames_requested),
                                   resampled_buffer.data() + i * frames_requested);
        }

        mGainFilter.process(resampled_buffer.data(), resampled_buffer.size());

        std::vector<float> interleaved_output(frames_requested * mOutputFormat.channelCount);

        if (mInputFormat.channelCount != mOutputFormat.channelCount) {
            mixChannels(resampled_buffer.data(), frames_requested,
                        mInputFormat.channelCount,
                        interleaved_output.data(),
                        mOutputFormat.channelCount);
        } else {
            for (size_t frame = 0; frame < frames_requested; ++frame) {
                for (size_t ch = 0; ch < mOutputFormat.channelCount; ++ch) {
                    interleaved_output[frame * mOutputFormat.channelCount + ch] =
                        resampled_buffer[ch * frames_requested + frame];
                }
            }
        }

        if (mOutputFormat.sampleFormat != ASampleFormat::F32) {
            aui::audio::convertSampleFormat(ASampleFormat::F32, mOutputFormat.sampleFormat,
                                            reinterpret_cast<const char*>(interleaved_output.data()),
                                            reinterpret_cast<char*>(dst.data()), frames_requested * mOutputFormat.channelCount);
        } else {
            std::memcpy(dst.data(), interleaved_output.data(), frames_requested * mOutputFormat.channelCount * sizeof(float));
        }

        return frames_requested * mOutputFormat.channelCount * aui::audio::bytesPerSample(mOutputFormat.sampleFormat);
    }
}

ASampleFormat AAudioResampler::outputSampleFormat() {
    return aui::audio::platform::requested_sample_format;
}

void AAudioResampler::setVolume(aui::audio::VolumeLevel volume) noexcept {
    mGainFilter.setVolume(volume);
}

void AAudioResampler::readCallback(size_t channel, int frames, float* destination) {
    if (channel >= mChannelBuffers.size()) {
        std::memset(destination, 0, frames * sizeof(float));
        return;
    }

    auto& channelBuffer = mChannelBuffers[channel];
    size_t framesToRead = frames;
    size_t offset = 0;

    while (framesToRead > 0) {
        if (channelBuffer.empty()) {
            readFrame();
            if (channelBuffer.empty()) {
                std::memset(destination + offset, 0, framesToRead * sizeof(float));
                break;
            }
        }

        size_t available = channelBuffer.size();
        size_t toCopy = std::min(available, framesToRead);

        std::memcpy(destination + offset, channelBuffer.data(), toCopy * sizeof(float));
        channelBuffer.erase(channelBuffer.begin(), channelBuffer.begin() + toCopy);

        framesToRead -= toCopy;
        offset += toCopy;
    }
}

void AAudioResampler::readFrame() {
    size_t bytesCount = mRequestFrames * aui::audio::bytesPerSample(mInputFormat.sampleFormat) * mInputFormat.channelCount;
    std::vector<char> readBuffer(bytesCount, 0);
    mSource->read(readBuffer.data(), bytesCount);

    size_t sampleCount = bytesCount / aui::audio::bytesPerSample(mInputFormat.sampleFormat);
    if (mInputFormat.sampleFormat != ASampleFormat::F32) {
        std::vector<char> floatBuffer(sampleCount * sizeof(float));
        aui::audio::convertSampleFormat(mInputFormat.sampleFormat, ASampleFormat::F32,
                                      readBuffer.data(), floatBuffer.data(), sampleCount);
        readBuffer = std::move(floatBuffer);
    }

    const float* samples = reinterpret_cast<const float*>(readBuffer.data());
    size_t frames = sampleCount / mInputFormat.channelCount;
    if (mChannelBuffers.size() < mInputFormat.channelCount) {
        mChannelBuffers.resize(mInputFormat.channelCount);
    }

    for (size_t channel = 0; channel < mInputFormat.channelCount; ++channel) {
        auto& buffer = mChannelBuffers[channel];
        buffer.reserve(buffer.size() + frames);
        for (size_t i = 0; i < frames; ++i) {
            buffer.push_back(samples[i * mInputFormat.channelCount + channel]);
        }
    }
}

void AAudioResampler::mixChannels(const float* input, size_t inputFrames, size_t inChannels,
                                    float* output, size_t outChannels) {
    if (inChannels == outChannels) {
        std::memcpy(output, input, inputFrames * inChannels * sizeof(float));
        return;
    }

    if (inChannels == 1 && outChannels == 2) {
        for (size_t frame = 0; frame < inputFrames; ++frame) {
            float sample = input[frame];
            output[frame * 2] = sample;
            output[frame * 2 + 1] = sample;
        }
    } else if (inChannels == 2 && outChannels == 1) {
        for (size_t frame = 0; frame < inputFrames; ++frame) {
            float left = input[frame * 2];
            float right = input[frame * 2 + 1];
            output[frame] = (left + right) * 0.5f;
        }
    } else {
        size_t minChannels = std::min(inChannels, outChannels);
        for (size_t frame = 0; frame < inputFrames; ++frame) {
            for (size_t ch = 0; ch < outChannels; ++ch) {
                if (ch < minChannels) {
                    output[frame * outChannels + ch] =
                        input[frame * inChannels + ch];
                } else {
                    output[frame * outChannels + ch] =
                        input[frame * inChannels + (minChannels - 1)];
                }
            }
        }
    }
}
