#pragma once

#include <type_traits>
#include <cstdint>
#include <cstddef>
#include "AUI/Common/AException.h"

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

    /**
     * @brief Signed 32-bit integer.
     */
    I32,

    /**
     * @brief 32-bit IEEE floating point.
     */
    F32,
};

namespace aui::audio {
    constexpr size_t bytesPerSample(ASampleFormat format) {
        switch (format) {
            case ASampleFormat::I16:
                return 2;

            case ASampleFormat::I24:
                return 3;

            case ASampleFormat::F32:
            case ASampleFormat::I32:
                return 4;
        }

        throw AException("Wrong sample format");
    }
}
