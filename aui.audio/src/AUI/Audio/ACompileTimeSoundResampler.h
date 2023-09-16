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
}

/**
 * @brief Implements audio mixing and resampling for ASoundResampler in compile time.
 */
template<ASampleFormat in, ASampleFormat out = aui::audio::DEFAULT_OUTPUT_FORMAT>
class ACompileTimeSoundResampler {
public:
    explicit ACompileTimeSoundResampler(std::span<std::byte> destination) noexcept:
            mDestinationBufferBegin(destination.data()),
            mDestinationBufferEnd(destination.data() + destination.size()),
            mDestinationBufferIt(mDestinationBufferBegin)
    { }

    inline void commitSample(aui::audio::util::sample_type_t<in> sample) {
        assert(("buffer overrun", mDestinationBufferIt <= mDestinationBufferEnd));
        //use int64_t for overflow preverting
        int64_t newSample = int64_t(aui::audio::util::sample_cast<out, in>(sample)) +
                            int64_t(aui::audio::util::extractSample<out>(mDestinationBufferIt));
        newSample = glm::clamp(newSample, MIN_VAL, MAX_VAL);
        aui::audio::util::pushSample<out>(newSample, mDestinationBufferIt);
        mDestinationBufferIt += aui::audio::util::size_bytes<out>();
    }


    [[nodiscard]]
    size_t remainingSampleCount() const {
        return (mDestinationBufferEnd - mDestinationBufferIt) / aui::audio::util::size_bytes<out>();
    }

    inline void commitAllSamples(const _<ISoundInputStream>& is) {
        std::byte buf[BUFFER_SIZE];
        while (remainingSampleCount() > 0) {
            auto toRead = std::min(size_t(remainingSampleCount() * aui::audio::util::size_bytes<in>()), sizeof(buf));
            size_t r = is->read(reinterpret_cast<char*>(buf), toRead);
            if (r == 0) {
                break;
            }
            std::byte* end = buf + r;
            for (std::byte* it = buf; it + aui::audio::util::size_bytes<in>() <= end; it += aui::audio::util::size_bytes<in>()) {
                commitSample(resampler(aui::audio::util::extractSample<in>(it)));
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


    using input_t = aui::audio::util::sample_type<in>;
    using output_t = aui::audio::util::sample_type<out>;

    static constexpr int64_t MIN_VAL = std::numeric_limits<typename output_t::type>::min();
    static constexpr int64_t MAX_VAL = std::numeric_limits<typename output_t::type>::max();
    static constexpr size_t BUFFER_SIZE = 0x3000;

private:
    std::byte* mDestinationBufferBegin;
    std::byte* mDestinationBufferEnd;
    std::byte* mDestinationBufferIt;
};