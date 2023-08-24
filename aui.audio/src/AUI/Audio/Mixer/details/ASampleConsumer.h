#pragma once

#include <cstdint>
#include "SampleFormat.h"
#include "AUI/Audio/Sound/ISoundStream.h"

namespace {
    template<int power, typename T>
    constexpr T multByPowerOf2(T value) {
        if constexpr(power > 0) {
            return value << power;
        } else {
            return value >> -power;
        }
    }

    template<SampleFormat f>
    struct sample_type;

    template<>
    struct sample_type<SampleFormat::I16> {
        using type = int16_t;
        constexpr static int size_bits = 16;
    };

    template<>
    struct sample_type<SampleFormat::I24> {
        using type = int32_t;
        constexpr static int size_bits = 24;
    };

    template<SampleFormat f>
    constexpr int size_bytes() {
        return sample_type<f>::size_bits / 8;
    }

    template<SampleFormat f>
    using sample_type_t = typename sample_type<f>::type;

#pragma pack(push, 1)
    template<SampleFormat f>
    struct packed_accessor {
        sample_type_t<f> value : sample_type<f>::size_bits;
        unsigned _pad : (32 - sample_type<f>::size_bits);
    };
#pragma pack(pop)
}

class ASampleConsumer {
public:
    ASampleConsumer(char* destinationBufferBegin, char* destinationBufferEnd) :
            mDestinationBufferBegin(destinationBufferBegin), mDestinationBufferEnd(destinationBufferEnd),
            mDestinationBufferIt(destinationBufferBegin)
    { }

    template<SampleFormat f>
    inline void commitSample(sample_type_t<f> sample) {
        assert(("buffer overrun", mDestinationBufferIt <= mDestinationBufferEnd));
        output_t::type sampleResampled = multByPowerOf2<output_t::size_bits - sample_type<f>::size_bits>(sample);
        auto& accessor = *reinterpret_cast<packed_accessor<OUTPUT_FORMAT>*>(mDestinationBufferIt);
        accessor.value = glm::clamp(int32_t(accessor.value) + int32_t(sampleResampled), MIN_VAL, MAX_VAL);
        mDestinationBufferIt += size_bytes<OUTPUT_FORMAT>();
    }


    [[nodiscard]]
    size_t remainingSampleCount() const {
        return (mDestinationBufferEnd - mDestinationBufferIt) / size_bytes<OUTPUT_FORMAT>();
    }

    template<SampleFormat f>
    inline void commitAllSamples(const _<ISoundStream>& is) {
        char buf[0x1000 * 3];
        while (remainingSampleCount() > 0) {
            size_t r = is->read(buf, std::min(size_t(remainingSampleCount() * size_bytes<f>()), sizeof(buf)));
            if (r == 0) {
                break;
            }
            char* end = buf + r;
            for (char* it = buf; it + size_bytes<f>() <= end; it += size_bytes<f>()) {
                auto value = *reinterpret_cast<packed_accessor<f>*>(it);
                commitSample<f>(value.value);
            }
        }
    }

    [[nodiscard]]
    bool isCommitted() const {
        return mDestinationBufferBegin != mDestinationBufferIt;
    }

    [[nodiscard]]
    size_t writtenSize() const {
        return mDestinationBufferIt - mDestinationBufferBegin;
    }


#if AUI_PLATFORM_ANDROID || AUI_PLATFORM_APPLE
    static constexpr SampleFormat OUTPUT_FORMAT = SampleFormat::I16;
#else
    static constexpr SampleFormat OUTPUT_FORMAT = SampleFormat::I24;
#endif
    using output_t = sample_type<OUTPUT_FORMAT>;

    static constexpr int32_t MIN_VAL = -multByPowerOf2<output_t::size_bits - 1>(1);
    static constexpr int32_t MAX_VAL = multByPowerOf2<output_t::size_bits - 1>(1) - 1;

private:
    char* mDestinationBufferBegin;
    char* mDestinationBufferEnd;
    char* mDestinationBufferIt;


};