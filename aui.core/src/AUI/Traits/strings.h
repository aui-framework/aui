/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2025 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <AUI/Common/AString.h>
#include <AUI/Common/AStringVector.h>
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
    inline AString format(std::string_view f, Args&&... args) {
        return fmt::format(fmt::runtime(f), detail::fmt<std::decay_t<Args>>::process(std::forward<Args>(args))...);
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
 * @brief String literal format caller (see example) (https://fmt.dev/11.1/syntax/)
 * @ingroup core
 * @details
 * ```cpp
 * "Hello {}!"_format("world") // -> "Hello world!"
 * "test {:.2}"_format(1.2f)   // -> "test 1.2"
 * ```
 */
inline AStringFormatHelper operator""_format(const char* str, size_t len)
{
    return {std::string_view(str, len)};
}
