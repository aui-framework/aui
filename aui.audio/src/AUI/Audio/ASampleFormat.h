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
