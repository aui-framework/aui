#include "AAudioResampler.h"
#include "Platform/RequestedAudioFormat.h"
#include "SincResampler.h"
#include "Util.h"

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
    mOutputFormat.sampleFormat = ASampleFormat::F32;

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
    mChannelBuffers.clear();
}

size_t AAudioResampler::read(std::span<std::byte> dst) {
    if (!mSource) return 0;

    size_t samples_requested = dst.size() / sizeof(float);
    size_t frames_requested = samples_requested / mOutputFormat.channelCount;

    if (mChannels.empty()) {
        size_t input_frames_to_read = frames_requested;
        size_t input_samples_to_read = input_frames_to_read * mInputFormat.channelCount;
        size_t bytes_to_read = input_samples_to_read * aui::audio::bytesPerSample(mInputFormat.sampleFormat);

        aui::impl::reserveVector(mReadBuffer, bytes_to_read);
        size_t bytes_read = mSource->read(mReadBuffer.data(), bytes_to_read);

        if (bytes_read == 0) {
            return 0;
        }

        size_t input_samples_read = bytes_read / aui::audio::bytesPerSample(mInputFormat.sampleFormat);
        size_t input_frames_read = input_samples_read / mInputFormat.channelCount;

        float* processing_data;

        if (mInputFormat.sampleFormat != ASampleFormat::F32) {
            aui::impl::reserveVector(mConvertedBuffer, input_samples_read * sizeof(float));
            aui::audio::convertSampleFormat(mInputFormat.sampleFormat, ASampleFormat::F32,
                                          mReadBuffer.data(), mConvertedBuffer.data(), input_samples_read);
            processing_data = reinterpret_cast<float*>(mConvertedBuffer.data());
        } else {
            processing_data = reinterpret_cast<float*>(mReadBuffer.data());
        }

        mGainFilter.process(processing_data, input_samples_read);

        float* output_data;
        size_t output_samples;

        if (mInputFormat.channelCount != mOutputFormat.channelCount) {
            output_samples = input_frames_read * mOutputFormat.channelCount;
            aui::impl::reserveVector(mMixingBuffer, output_samples * sizeof(float));

            mixChannels(processing_data, input_frames_read,
                          mInputFormat.channelCount,
                          reinterpret_cast<float*>(mMixingBuffer.data()),
                          mOutputFormat.channelCount);

            output_data = reinterpret_cast<float*>(mMixingBuffer.data());
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
        if (mChannelBuffers[0].empty()) {
            readFrame();
            bool stillEmpty = std::all_of(mChannelBuffers.begin(), mChannelBuffers.end(),
                                          [](auto &b) { return b.empty(); });
            if (stillEmpty) return 0;
        }

        aui::impl::reserveVector(mResamplingBuffer, frames_requested * mInputFormat.channelCount);
        for (size_t i = 0; i < mInputFormat.channelCount; ++i) {
            mChannels[i]->Resample(static_cast<int>(frames_requested),
                                   mResamplingBuffer.data() + i * frames_requested);
        }

        mGainFilter.process(mResamplingBuffer.data(), frames_requested * mInputFormat.channelCount);

        size_t mixing_bytes = frames_requested * mOutputFormat.channelCount * sizeof(float);
        aui::impl::reserveVector(mMixingBuffer, mixing_bytes);

        if (mInputFormat.channelCount != mOutputFormat.channelCount) {
            mixChannels(mResamplingBuffer.data(), frames_requested,
                        mInputFormat.channelCount,
                        reinterpret_cast<float*>(mMixingBuffer.data()),
                        mOutputFormat.channelCount);
        } else {
            float* mixingFloats = reinterpret_cast<float*>(mMixingBuffer.data());
            for (size_t ch = 0; ch < mOutputFormat.channelCount; ++ch) {
                for (size_t frame = 0; frame < frames_requested; ++frame) {
                    mixingFloats[frame * mOutputFormat.channelCount + ch] = mResamplingBuffer[ch * frames_requested + frame];
                }
            }
        }

        std::memcpy(dst.data(), mMixingBuffer.data(), frames_requested * mOutputFormat.channelCount * sizeof(float));

        return frames_requested * mOutputFormat.channelCount * aui::audio::bytesPerSample(mOutputFormat.sampleFormat);
    }
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
    aui::impl::reserveVector(mReadBuffer, bytesCount);
    size_t bytesRead = mSource->read(mReadBuffer.data(), bytesCount);

    if (bytesRead == 0) {
        return;
    }

    const char* samplesBytes;

    size_t sampleCount = bytesCount / aui::audio::bytesPerSample(mInputFormat.sampleFormat);
    if (mInputFormat.sampleFormat != ASampleFormat::F32) {
        aui::impl::reserveVector(mConvertedBuffer, sampleCount * sizeof(float));
        aui::audio::convertSampleFormat(mInputFormat.sampleFormat, ASampleFormat::F32,
                                      mReadBuffer.data(), mConvertedBuffer.data(), sampleCount);
        samplesBytes = mConvertedBuffer.data();
    } else {
        samplesBytes = mReadBuffer.data();
    }

    const float* samples = reinterpret_cast<const float*>(samplesBytes);
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
