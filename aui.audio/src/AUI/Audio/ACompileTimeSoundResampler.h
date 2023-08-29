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
#include <AUI/Audio/ISoundInputStream.h>
#include <AUI/Traits/platform.h>

namespace aui::audio {

    /**
     * @brief Default output format for the current platform.
     * @ingroup audio
     */
    static constexpr ASampleFormat DEFAULT_OUTPUT_FORMAT = aui::platform::current::is_mobile() ? ASampleFormat::I16
                                                                                               : ASampleFormat::I24;

    namespace impl {
        template<int power, typename T>
        constexpr T multByPowerOf2(T value) {
            if constexpr (power > 0) {
                return value << power;
            } else {
                return value >> -power;
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

        template<ASampleFormat f>
        constexpr int size_bytes() {
            return sample_type<f>::size_bits / 8;
        }

        template<ASampleFormat f>
        using sample_type_t = typename sample_type<f>::type;

#pragma pack(push, 1)
        template<ASampleFormat f>
        struct packed_accessor {
            sample_type_t<f> value: sample_type<f>::size_bits;
            unsigned _pad: (32 - sample_type<f>::size_bits);
        };
#pragma pack(pop)
    }
}

/**
 * @brief Implements audio mixing and resampling for ASoundResampler in compile time.
 */
template<ASampleFormat in, ASampleFormat out = aui::audio::DEFAULT_OUTPUT_FORMAT>
class ACompileTimeSoundResampler {
public:
    ACompileTimeSoundResampler(std::span<std::byte> destination) noexcept:
            mDestinationBufferBegin(destination.data()),
            mDestinationBufferEnd(destination.data() + destination.size()),
            mDestinationBufferIt(mDestinationBufferBegin)
    { }

    inline void commitSample(aui::audio::impl::sample_type_t<in> sample) {
        assert(("buffer overrun", mDestinationBufferIt <= mDestinationBufferEnd));
        typename output_t::type sampleResampled = aui::audio::impl::multByPowerOf2<output_t::size_bits - aui::audio::impl::sample_type<in>::size_bits, typename output_t::type>(sample);
        auto& accessor = *reinterpret_cast<aui::audio::impl::packed_accessor<out>*>(mDestinationBufferIt);
        accessor.value = glm::clamp(int32_t(accessor.value) + int32_t(sampleResampled), MIN_VAL, MAX_VAL);
        mDestinationBufferIt += aui::audio::impl::size_bytes<out>();
    }


    [[nodiscard]]
    size_t remainingSampleCount() const {
        return (mDestinationBufferEnd - mDestinationBufferIt) / aui::audio::impl::size_bytes<out>();
    }

    inline void commitAllSamples(const _<ISoundInputStream>& is) {
        std::byte buf[0x1000 * 3];
        while (remainingSampleCount() > 0) {
            auto toRead = std::min(size_t(remainingSampleCount() * aui::audio::impl::size_bytes<in>()), sizeof(buf));
            size_t r = is->read(reinterpret_cast<char*>(buf), toRead);
            if (r == 0) {
                break;
            }
            std::byte* end = buf + r;
            for (std::byte* it = buf; it + aui::audio::impl::size_bytes<in>() <= end; it += aui::audio::impl::size_bytes<in>()) {
                auto value = *reinterpret_cast<aui::audio::impl::packed_accessor<in>*>(it);
                commitSample(value.value);
            }
        }
    }

    [[nodiscard]]
    bool isFull() const {
        return mDestinationBufferBegin != mDestinationBufferIt;
    }

    [[nodiscard]]
    size_t writtenSize() const {
        return mDestinationBufferIt - mDestinationBufferBegin;
    }


    using output_t = aui::audio::impl::sample_type<out>;

    static constexpr int32_t MIN_VAL = -aui::audio::impl::multByPowerOf2<output_t::size_bits - 1>(1);
    static constexpr int32_t MAX_VAL = aui::audio::impl::multByPowerOf2<output_t::size_bits - 1>(1) - 1;

private:
    std::byte* mDestinationBufferBegin;
    std::byte* mDestinationBufferEnd;
    std::byte* mDestinationBufferIt;


};