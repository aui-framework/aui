#pragma once

#include <memory>

namespace aui {
    /**
     * \brief Fill with zeros object's memory.
     * \tparam T object type
     * \param data object itself
     */
    template<typename T>
    inline void zero(T& data) {
        memset(&data, 0, sizeof(T));
    }
}