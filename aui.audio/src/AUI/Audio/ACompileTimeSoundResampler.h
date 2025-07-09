/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstdint>
#include <span>
#include <utility>
#include <AUI/Audio/ASampleFormat.h>
#include <AUI/Audio/ISoundInputStream.h>
#include <AUI/Audio/Platform/RequestedAudioFormat.h>
#include <AUI/Audio/ASampleRateConverter.h>
#include <AUI/Audio/VolumeLevel.h>

namespace aui::audio::impl {
template <int power, typename T>
constexpr T multByPowerOf2(T value) {
    if constexpr (power > 0) {
        return value << power;
    } else {
        return value >> -power;
    }
}

template <int shift, typename T>
constexpr T logicalShift(T value) {
    if constexpr (std::is_floating_point_v<T>) {
        return value;
    } else {
        if constexpr (shift > 0) {
            return static_cast<T>(static_cast<std::make_unsigned_t<T>>(value) << shift);
        } else {
            return static_cast<T>(static_cast<std::make_unsigned_t<T>>(value) >> -shift);
        }
    }
}

template <ASampleFormat f>
struct sample_type;

template <>
struct sample_type<ASampleFormat::I16> {
    using type = int16_t;
    constexpr static int size_bits = 16;
};

template <>
struct sample_type<ASampleFormat::I24> {
    using type = int32_t;
    constexpr static int size_bits = 24;
};

template <>
struct sample_type<ASampleFormat::I32> {
    using type = int32_t;
    constexpr static int size_bits = 32;
};

template <>
struct sample_type<ASampleFormat::F32> {
    using type = float;
    constexpr static int size_bits = 32;
};

template <ASampleFormat f>
constexpr int size_bytes() {
    return sample_type<f>::size_bits / 8;
}

template <ASampleFormat f>
using sample_type_t = typename sample_type<f>::type;

template <ASampleFormat f>
constexpr int type_size() {
    return sizeof(sample_type_t<f>);
}

template <ASampleFormat f>
constexpr int type_size_bits() {
    return type_size<f>() * 8;
}

template <ASampleFormat to, ASampleFormat from>
constexpr sample_type_t<to> sample_cast(sample_type_t<from> sample) {
    using FromT = sample_type_t<from>;
    if constexpr (std::is_floating_point_v<FromT>) {
        return glm::mix(
            FromT(std::numeric_limits<sample_type_t<to>>::min()), FromT(std::numeric_limits<sample_type_t<to>>::max()),
            (sample + 1) / 2.f);
    } else {
        if constexpr (type_size<to>() > type_size<from>()) {
            return logicalShift<type_size_bits<to>() - type_size_bits<from>()>(static_cast<sample_type_t<to>>(sample));
        }
        return logicalShift<type_size_bits<to>() - type_size_bits<from>()>(sample);
    }
}

#pragma pack(push, 1)
template <ASampleFormat f>
struct packed_accessor {
    sample_type_t<f> value : sample_type<f>::size_bits;
    unsigned _pad : (32 - sample_type<f>::size_bits);
};
#pragma pack(pop)

template <ASampleFormat f>
    requires requires { sample_type<f>::size_bits >= 32; }
struct packed_accessor<f> {
    sample_type_t<f> value;
};

template <ASampleFormat f>
sample_type_t<f> extractSample(std::byte* src) {
    return logicalShift<type_size_bits<f>() - sample_type<f>::size_bits>(
        reinterpret_cast<packed_accessor<f>*>(src)->value);
}

template <ASampleFormat f>
void pushSample(sample_type_t<f> sample, std::byte* dst) {
    reinterpret_cast<packed_accessor<f>*>(dst)->value =
        logicalShift<sample_type<f>::size_bits - type_size_bits<f>()>(sample);
}
}   // namespace aui::audio::impl

struct Transaction {
    std::byte* destinationBufferBegin = nullptr;
    std::byte* destinationBufferEnd = nullptr;
    std::byte* destinationBufferIt = nullptr;
    AOptional<aui::audio::VolumeLevel> volumeLevel;

    [[nodiscard]]
    bool isFull() const {
        return destinationBufferBegin != destinationBufferIt;
    }

    [[nodiscard]]
    size_t writtenSize() const {
        return destinationBufferIt - destinationBufferBegin;
    }

    template <ASampleFormat sample_out>
    [[nodiscard]]
    size_t remainingSampleCount() const {
        return (destinationBufferEnd - destinationBufferIt) / aui::audio::impl::size_bytes<sample_out>();
    }
};

template <
    ASampleFormat sample_in, AChannelFormat channels_in,
    ASampleFormat sample_out = aui::audio::platform::requested_sample_format,
    AChannelFormat channels_out = aui::audio::platform::requested_channels_format>
class ACompileTimeSoundResampler {
public:
    explicit ACompileTimeSoundResampler(_<ISoundInputStream> source) noexcept
      : mInputSampleRate(source->info().sampleRate)
      , mConverter(aui::audio::platform::requested_sample_rate, std::move(source)) {}

    template <ASampleFormat format>
    inline void commitSample(Transaction& transaction, aui::audio::impl::sample_type_t<format> sample) {
        AUI_ASSERTX(transaction.destinationBufferIt <= transaction.destinationBufferEnd, "buffer overrun");
        // use int64_t for overflow preventing
        int64_t newSample = int64_t(aui::audio::impl::sample_cast<sample_out, format>(sample));
        if (transaction.volumeLevel) {
            newSample = (*transaction.volumeLevel * newSample) / aui::audio::VolumeLevel::MAX;
        }
        newSample += int64_t(aui::audio::impl::extractSample<sample_out>(transaction.destinationBufferIt));
        newSample = glm::clamp(newSample, MIN_VAL, MAX_VAL);
        aui::audio::impl::pushSample<sample_out>(newSample, transaction.destinationBufferIt);
        transaction.destinationBufferIt += aui::audio::impl::size_bytes<sample_out>();
    }

    constexpr size_t canReadSamples(size_t canPushSamples) {
        return (canPushSamples / size_t(channels_out)) * size_t(channels_in);
    }

    void commitAllSamples(Transaction& transaction) {
        int deadbeef1 = 0xdeadbeef;
        std::byte buf[BUFFER_SIZE];
        int deadbeef2 = 0xdeadbeef;
        while (auto remSampleCount = transaction.remainingSampleCount<sample_out>()) {
            size_t samplesToRead = canReadSamples(remSampleCount);
            if (samplesToRead == 0) {
                break;
            }
            size_t r;
            if (mInputSampleRate == aui::audio::platform::requested_sample_rate) {
                std::span dst(buf, std::min(aui::audio::impl::size_bytes<sample_in>() * samplesToRead, sizeof(buf)));
                r = mConverter.source()->read(dst);
#if AUI_DEBUG
                AUI_ASSERTX(r <= dst.size(), "result larger than supplied buffer?");
                AUI_ASSERTX(deadbeef1 == 0xdeadbeef, "stack corruption");
                AUI_ASSERTX(deadbeef2 == 0xdeadbeef, "stack corruption");
#endif
                iterateOverBuffer<sample_in>(transaction, buf, buf + r);
            } else {
                static constexpr auto conv_sample_format = ASampleRateConverter::outputSampleFormat();
                std::span dst(
                    buf, std::min(aui::audio::impl::size_bytes<conv_sample_format>() * samplesToRead, sizeof(buf)));
                r = mConverter.convert(dst);
#if AUI_DEBUG
                AUI_ASSERTX(r <= dst.size(), "result larger than supplied buffer?");
#endif
                iterateOverBuffer<conv_sample_format>(transaction, buf, buf + r);
#if AUI_DEBUG
                AUI_ASSERTX(deadbeef1 == 0xdeadbeef, "stack corruption");
                AUI_ASSERTX(deadbeef2 == 0xdeadbeef, "stack corruption");
#endif
            }

            if (r == 0) {
                break;
            }
        }
    }

    template <ASampleFormat format>
    void iterateOverBuffer(Transaction& transaction, std::byte* begin, std::byte* end) {
        static constexpr size_t stepSize = static_cast<size_t>(channels_in) * aui::audio::impl::size_bytes<format>();
        for (std::byte* it = begin; it + stepSize <= end; it += stepSize) {
            if constexpr (channels_in == channels_out) {
                for (size_t i = 0; i < static_cast<size_t>(channels_in); i++) {
                    commitSample<format>(
                        transaction,
                        aui::audio::impl::extractSample<format>(it + i * aui::audio::impl::size_bytes<format>()));
                }
            } else {
                if constexpr (channels_in == AChannelFormat::MONO) {
                    // mono to stereo resampling
                    auto sample = aui::audio::impl::extractSample<format>(it);
                    commitSample<format>(transaction, sample);
                    commitSample<format>(transaction, sample);
                } else {
                    // TODO implement stereo to mono resampling
                }
            }
        }
    }

    using input_t = aui::audio::impl::sample_type<sample_in>;
    using output_t = aui::audio::impl::sample_type<sample_out>;

    static constexpr int64_t MIN_VAL = std::numeric_limits<typename output_t::type>::min();
    static constexpr int64_t MAX_VAL = std::numeric_limits<typename output_t::type>::max();
    static constexpr size_t BUFFER_SIZE = 0x3000;

private:
    std::uint32_t mInputSampleRate;
    ASampleRateConverter mConverter;
};
