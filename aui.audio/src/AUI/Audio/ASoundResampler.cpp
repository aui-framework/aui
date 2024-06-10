#include "ASoundResampler.h"
#include "AUI/Audio/ACompileTimeSoundResampler.h"
#include "AUI/Audio/IAudioPlayer.h"

namespace aui::audio::impl {
    template<ASampleFormat sample_in, AChannelFormat channels_in>
    class Resampler final : public ResamplerBase {
    public:
        explicit Resampler(_<ISoundInputStream> source) : mResampler(std::move(source)) {
        }

        size_t resample(std::span<std::byte> dst, aui::audio::VolumeLevel volume) override {
            if (volume != aui::audio::VolumeLevel::MAX) {
                mResampler.setVolume(volume);
            }
            mResampler.setDestination(dst);
            mResampler.commitAllSamples();
            return mResampler.writtenSize();
        }

    private:
        ACompileTimeSoundResampler<sample_in, channels_in> mResampler;
    };

    template<ASampleFormat format>
    struct SampleFormatHelper {
        static constexpr ASampleFormat value = format;
    };

    template<AChannelFormat format>
    struct ChannelFormatHelper {
        static constexpr AChannelFormat value = format;
    };

    using I16Helper = SampleFormatHelper<ASampleFormat::I16>;
    using I24Helper = SampleFormatHelper<ASampleFormat::I24>;
    using I32Helper = SampleFormatHelper<ASampleFormat::I32>;
    using MonoHelper = ChannelFormatHelper<AChannelFormat::MONO>;
    using StereoHelper = ChannelFormatHelper<AChannelFormat::STEREO>;

    constexpr size_t SAMPLE_FORMAT_INDEX = 0;
    constexpr size_t CHANNEL_FORMAT_INDEX = 1;
    constexpr size_t RESAMPLER_ARGS_COUNT = 2;

    template<typename ...Args>
    _unique<ResamplerBase> resolveResampler(_<ISoundInputStream> source) {
        static_assert(sizeof...(Args) <= RESAMPLER_ARGS_COUNT);

        if constexpr (sizeof...(Args) == SAMPLE_FORMAT_INDEX) {
            switch (source->info().sampleFormat) {
                case ASampleFormat::I16:
                    return resolveResampler<Args..., I16Helper>(std::move(source));
                case ASampleFormat::I24:
                    return resolveResampler<Args..., I24Helper>(std::move(source));
                case ASampleFormat::I32:
                    return resolveResampler<Args..., I32Helper>(std::move(source));
            }
            throw AException("invalid input sample format = {}"_format(int(source->info().sampleFormat)));
        }

        if constexpr (sizeof...(Args) == CHANNEL_FORMAT_INDEX) {
            switch (source->info().channelCount) {
                case AChannelFormat::MONO:
                    return resolveResampler<Args..., MonoHelper>(std::move(source));
                case AChannelFormat::STEREO:
                    return resolveResampler<Args..., StereoHelper>(std::move(source));
            }
            throw AException("invalid input channel count = {}"_format(int(source->info().channelCount)));
        }

        if constexpr (sizeof...(Args) == RESAMPLER_ARGS_COUNT) {
            return std::make_unique<Resampler<Args::value...>>(std::move(source));
        }
    }
}

ASoundResampler::ASoundResampler(_<ISoundInputStream> sourceStream) noexcept : mSourceStream(std::move(sourceStream)) {
    mResampler = aui::audio::impl::resolveResampler<>(mSourceStream);
}

size_t ASoundResampler::read(char *dst, size_t size) {
    std::span<std::byte> destination(reinterpret_cast<std::byte*>(dst), size);
    return mResampler->resample(destination, mVolume);
}

AAudioFormat ASoundResampler::info() {
    return aui::audio::platform::requested_format;
}

void ASoundResampler::setVolume(aui::audio::VolumeLevel volume) noexcept {
    mVolume = volume;
}
