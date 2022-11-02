/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

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
