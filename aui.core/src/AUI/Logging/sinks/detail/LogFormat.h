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

#include <AUI/Common/AStringView.h>
#include <chrono>
#include <ctime>
#include <fmt/color.h>
#include <fmt/format.h>
#include <string>

namespace aui::detail::log {

    inline AStringView levelCStr(int level) {
        switch (level) {
            case 0: return "INFO";
            case 1: return "WARN";
            case 2: return "ERR";
            case 3: return "DEBUG";
            case 4: return "TRACE";
        }
        return "UNKNOWN";
    }

    /// Formats a log level name with ANSI colors, or plain when @p useColors is false.
    inline std::string levelColored(int level, bool useColors) {
        if (!useColors)
            return std::string(levelCStr(level));
        switch (level) {
            case 0: return ::fmt::format(::fmt::fg(::fmt::color::green), "{}", "INFO");
            case 1: return ::fmt::format(::fmt::fg(::fmt::color::yellow), "{}", "WARN");
            case 2: return ::fmt::format(::fmt::fg(::fmt::color::red), "{}", "ERR");
            case 3: return ::fmt::format(::fmt::fg(::fmt::color::cyan), "{}", "DEBUG");
            case 4: return ::fmt::format(::fmt::fg(::fmt::color::gray), "{}", "TRACE");
        }
        return "UNKNOWN";
    }

    /// Formats a unix milliseconds timestamp as "HH:MM:SS".
    inline std::string formatTimestamp(long long timestampMs) {
        std::time_t seconds = static_cast<std::time_t>(timestampMs / 1000);
        std::tm tm;
#if defined(_WIN32)
        localtime_s(&tm, &seconds);
#else
        localtime_r(&seconds, &tm);
#endif
        char buf[16];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
        return buf;
    }
}
