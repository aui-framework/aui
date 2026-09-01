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

#include "AConsoleSink.h"
#include "detail/LogFormat.h"
#include <cstdio>
#include <fmt/format.h>
#include <fmt/color.h>

static std::string levelColored(int level, bool useColors) {
    if (!useColors)
        return aui::detail::log::levelCStr(level);
    switch (level) {
        case 0: return fmt::format(fmt::fg(fmt::color::green), "{}", "INFO");
        case 1: return fmt::format(fmt::fg(fmt::color::yellow), "{}", "WARN");
        case 2: return fmt::format(fmt::fg(fmt::color::red), "{}", "ERR");
        case 3: return fmt::format(fmt::fg(fmt::color::cyan), "{}", "DEBUG");
        case 4: return fmt::format(fmt::fg(fmt::color::gray), "{}", "TRACE");
    }
    return "UNKNOWN";
}

AConsoleSink::AConsoleSink(bool useColors) : mUseColors(useColors) {}

void AConsoleSink::write(const ALogMessage& message) {
    std::string consoleMsg;
    if (message.message.empty()) {
        consoleMsg = fmt::format("[{}][{}][{}]: {}",
                                 message.timestamp, message.threadName,
                                 levelColored(message.level, mUseColors),
                                 message.prefix);
    } else {
        consoleMsg = fmt::format("[{}][{}][{}][{}]: {}",
                                 message.timestamp, message.threadName, message.prefix,
                                 levelColored(message.level, mUseColors),
                                 message.message);
    }
    fputs(consoleMsg.c_str(), stdout);
    fputc('\n', stdout);
    fflush(stdout);
}

void AConsoleSink::flush() {
    fflush(stdout);
}

std::string_view AConsoleSink::name() const noexcept {
    return "console";
}