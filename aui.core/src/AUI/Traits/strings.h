// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <AUI/Common/AString.h>
#include <AUI/Common/AStringVector.h>
#include <AUI/IO/APath.h>
#include <glm/glm.hpp>
#include <cstring>
#include "types.h"

#include <fmt/format.h>

namespace aui {

    namespace detail {
        template<typename T, typename EnableIf = void>
        struct fmt {
            template<typename T2>
            static decltype(auto) process(T2&& arg) {
                return std::forward<T2>(arg);
            }
        };

        template<typename T>
        struct fmt<T, std::enable_if_t<std::is_base_of_v<AString, T>>> {
            template<typename T2>
            static decltype(auto) process(T2&& arg) {
                return arg.toStdString();
            }
        };
    }

    /**
     * @brief Formats string.
     * @return formatted string.
     * @details
     * AUI uses fmt library. See documentation at https://fmt.dev/latest/index.html
     */
    template <typename... Args>
    inline AString format(std::string_view format, Args&&... args) {
        return fmt::format(format, detail::fmt<std::decay_t<Args>>::process(std::forward<Args>(args))...);
    }
}

template<typename... Args>
inline AString AString::format(Args&& ... args) const {
    return aui::format(toStdString(), std::forward<Args>(args)...);
}


struct AStringFormatHelper {
    std::string_view string;

    template<typename... Args>
    inline AString operator()(Args&& ... args) {
        return aui::format(string, std::forward<Args>(args)...);
    }
};

/**
 * @brief String literal format caller (see example).
 * @ingroup core
 * @details
 * @code{cpp}
 * "Hello {}!"_format("world") // -> "Hello world!"
 * "test {:.2}"_format(1.2f)   // -> "test 1.2"
 * @endcode
 */
inline AStringFormatHelper operator"" _format(const char* str, size_t len)
{
    return {std::string_view(str, len)};
}
