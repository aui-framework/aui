#pragma once

#include <memory>

namespace aui {
    /**
     * \brief Заполнить нулями объект.
     * \tparam T тип объекта
     * \param data объект
     */
    template<typename T>
    inline void zero(T& data) {
        memset(&data, 0, sizeof(T));
    }
}