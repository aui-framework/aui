#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>

/**
 * @brief Sample formats supported for mixing.
 * @ingroup audio
 */
enum class ASampleFormat {
    /**
     * @brief Signed 16-bit integer.
     */
    I16,

    /**
     * @brief Signed 24-bit integer.
     */
    I24,
};

namespace aui::audio::util {
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
            return logicalShift<type_size_bits<to>() - type_size_bits<from>()>(static_cast<sample_type_t<to>>(sample));
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

    template<ASampleFormat f>
    sample_type_t<f> extractSample(std::byte* src) {
        return logicalShift<type_size_bits<f>() - sample_type<f>::size_bits>(reinterpret_cast<packed_accessor<f>*>(src)->value);
    }

    template<ASampleFormat f>
    void pushSample(sample_type_t<f> sample, std::byte* dst) {
        reinterpret_cast<packed_accessor<f>*>(dst)->value = logicalShift<sample_type<f>::size_bits - type_size_bits<f>()>(sample);
    }
}
