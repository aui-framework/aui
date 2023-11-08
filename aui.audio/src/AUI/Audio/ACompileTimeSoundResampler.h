//  AUI Framework - Declarative UI toolkit for modern C++20
//  Copyright (C) 2020-2023 Alex2772
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <cstdint>
#include <span>
#include "ASampleFormat.h"
#include "AUI/Audio/IAudioPlayer.h"
#include "AUI/Audio/ISoundInputStream.h"
#include "AUI/Audio/Platform/RequestedAudioFormat.h"
#include "ASampleRateConverter.h"

namespace aui::audio {
    namespace impl {
        template<int power, typename T>
        constexpr T multByPowerOf2(T value) {
            if constexpr (power > 0) {
                return value << power;
            } else {
                return value >> -power;
            }
        }

        template<int shift, typename T>
        constexpr T logicalShift(T value) {
            if constexpr (shift > 0) {
                return static_cast<T>(static_cast<std::make_unsigned_t<T>>(value) << shift);
            } else {
                return static_cast<T>(static_cast<std::make_unsigned_t<T>>(value) >> -shift);
            }
        }

        template<ASampleFormat f>
        struct sample_type;

        template<>
        struct sample_type<ASampleFormat::I16> {
            using type = int16_t;
            constexpr static int size_bits = 16;
        };

        template<>
        struct sample_type<ASampleFormat::I24> {
            using type = int32_t;
            constexpr static int size_bits = 24;
        };

        template<>
        struct sample_type<ASampleFormat::I32> {
            using type = int32_t;
            constexpr static int size_bits = 32;
        };

        template<ASampleFormat f>
        constexpr int size_bytes() {
            return sample_type<f>::size_bits / 8;
        }

        template<ASampleFormat f>
        using sample_type_t = typename sample_type<f>::type;

        template<ASampleFormat f>
        constexpr int type_size() {
            return sizeof(sample_type_t<f>);
        }

        template<ASampleFormat f>
        constexpr int type_size_bits() {
            return type_size<f>() * 8;
        }

        template<ASampleFormat to, ASampleFormat from>
        constexpr sample_type_t<to> sample_cast(sample_type_t<from> sample) {
            if constexpr (type_size<to>() > type_size<from>()) {
                return logicalShift<type_size_bits<to>() - type_size_bits<from>()>(
                        static_cast<sample_type_t<to>>(sample));
            }
            return logicalShift<type_size_bits<to>() - type_size_bits<from>()>(sample);
        }

#pragma pack(push, 1)
        template<ASampleFormat f>
        struct packed_accessor {
            sample_type_t<f> value: sample_type<f>::size_bits;
            unsigned _pad: (32 - sample_type<f>::size_bits);
        };
#pragma pack(pop)

        template<>
        struct packed_accessor<ASampleFormat::I32> {
            sample_type_t<ASampleFormat::I32> value: sample_type<ASampleFormat::I32>::size_bits;
        };

        template<ASampleFormat f>
        sample_type_t<f> extractSample(std::byte *src) {
            return logicalShift<type_size_bits<f>() - sample_type<f>::size_bits>(
                    reinterpret_cast<packed_accessor<f> *>(src)->value);
        }

        template<ASampleFormat f>
        void pushSample(sample_type_t<f> sample, std::byte *dst) {
            reinterpret_cast<packed_accessor<f> *>(dst)->value = logicalShift<
                    sample_type<f>::size_bits - type_size_bits<f>()>(sample);
        }
    }

}


/**
 * @brief Implements audio mixing and resampling for ASoundResampler in compile time.
 */
template<ASampleFormat sample_in, AChannelFormat channels_in,
        ASampleFormat sample_out = aui::audio::platform::requested_sample_format,
        AChannelFormat channels_out = aui::audio::platform::requested_channels_format>
class ACompileTimeSoundResampler {
public:
    explicit ACompileTimeSoundResampler(_<ISoundInputStream> source) noexcept :
            mInputSampleRate(source->info().sampleRate), mSource(source),
            mConverter(aui::audio::platform::requested_sample_rate, std::move(source)) {
    }

    void setVolume(IAudioPlayer::VolumeLevel volume) {
        mVolumeLevel = volume;
    }

    template<ASampleFormat format>
    inline void commitSample(aui::audio::impl::sample_type_t<format> sample) {
        assert(("buffer overrun", mDestinationBufferIt <= mDestinationBufferEnd));
        //use int64_t for overflow preventing
        int64_t newSample = int64_t(aui::audio::impl::sample_cast<sample_out, format>(sample));
        if (mVolumeLevel) {
            newSample = (*mVolumeLevel * newSample) / IAudioPlayer::VolumeLevel::MAX;
        }
        newSample += int64_t(aui::audio::impl::extractSample<sample_out>(mDestinationBufferIt));
        newSample = glm::clamp(newSample, MIN_VAL, MAX_VAL);
        aui::audio::impl::pushSample<sample_out>(newSample, mDestinationBufferIt);
        mDestinationBufferIt += aui::audio::impl::size_bytes<sample_out>();
    }

    [[nodiscard]]
    size_t remainingSampleCount() const {
        return (mDestinationBufferEnd - mDestinationBufferIt) / aui::audio::impl::size_bytes<sample_out>();
    }

    constexpr size_t canReadSamples(size_t canPushSamples) {
        return (canPushSamples / size_t(channels_out)) * size_t(channels_in);
    }

    inline void commitAllSamples() {
        std::byte buf[BUFFER_SIZE];
        while (auto remSampleCount = remainingSampleCount()) {
            size_t samplesToRead = canReadSamples(remainingSampleCount());
            size_t r;
            if (mInputSampleRate == aui::audio::platform::requested_sample_rate) {
                std::span dst(buf, std::min(aui::audio::impl::size_bytes<sample_in>() * samplesToRead, sizeof(buf)));
                r = mSource->read(dst);
                iterateOverBuffer<sample_in>(buf, buf + r);
            }
            else {
                static constexpr auto conv_sample_format = ASampleRateConverter::outputSampleFormat();
                std::span dst(buf, std::min(aui::audio::impl::size_bytes<conv_sample_format>() * samplesToRead, sizeof(buf)));
                r = mConverter.convert(dst);
                iterateOverBuffer<conv_sample_format>(buf, buf + r);
            }

            if (r == 0) {
                break;
            }
        }
    }

    template <ASampleFormat format>
    void iterateOverBuffer(std::byte* begin, std::byte* end) {
        static constexpr size_t stepSize = static_cast<size_t>(channels_in) * aui::audio::impl::size_bytes<format>();
        for (std::byte *it = begin; it + stepSize <= end; it += stepSize) {
            if constexpr (channels_in == channels_out) {
                for (size_t i = 0; i < static_cast<size_t>(channels_in); i++) {
                    commitSample<format>(aui::audio::impl::extractSample<format>(
                            it + i * aui::audio::impl::size_bytes<format>()));
                }
            } else {
                if constexpr (channels_in == AChannelFormat::MONO) {
                    //mono to stereo resampling
                    auto sample = aui::audio::impl::extractSample<format>(it);
                    commitSample<format>(sample);
                    commitSample<format>(sample);
                } else {
                    //TODO implement stereo to mono resampling
                }
            }
        }
    }

    void setDestination(std::span<std::byte> dst) {
        mDestinationBufferBegin = dst.data();
        mDestinationBufferEnd = dst.data() + dst.size();
        mDestinationBufferIt = mDestinationBufferBegin;
    }

    [[nodiscard]]
    bool isFull() const {
        return mDestinationBufferBegin != mDestinationBufferIt;
    }

    [[nodiscard]]
    size_t writtenSize() const {
        return mDestinationBufferIt - mDestinationBufferBegin;
    }

    using input_t = aui::audio::impl::sample_type<sample_in>;
    using output_t = aui::audio::impl::sample_type<sample_out>;

    static constexpr int64_t MIN_VAL = std::numeric_limits<typename output_t::type>::min();
    static constexpr int64_t MAX_VAL = std::numeric_limits<typename output_t::type>::max();
    static constexpr size_t BUFFER_SIZE = 0x3000;

private:
    std::byte *mDestinationBufferBegin = nullptr;
    std::byte *mDestinationBufferEnd = nullptr;
    std::byte *mDestinationBufferIt = nullptr;
    std::uint32_t mInputSampleRate;
    _<ISoundInputStream> mSource;
    ASampleRateConverter mConverter;
    AOptional<IAudioPlayer::VolumeLevel> mVolumeLevel;

};
