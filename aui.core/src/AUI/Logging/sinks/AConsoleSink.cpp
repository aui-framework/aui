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
#include <AUI/Core.h>

AConsoleSink::AConsoleSink(bool useColors) : mUseColors(useColors) {}

void AConsoleSink::write(const ALogMessage& message) {
    auto timestamp = aui::detail::log::formatTimestamp(message.timestampMs);
    std::string consoleMsg;
    if (message.message.empty()) {
        consoleMsg = fmt::format("[{}][{}][{}]: {}",
                                 timestamp, message.threadName,
                                 aui::detail::log::levelColored(message.level, mUseColors),
                                 message.prefix);
    } else {
        consoleMsg = fmt::format("[{}][{}][{}][{}]: {}",
                                 timestamp, message.threadName, message.prefix,
                                 aui::detail::log::levelColored(message.level, mUseColors),
                                 message.message);
    }
    fputs(consoleMsg.c_str(), stdout);
    fputc('\n', stdout);
    fflush(stdout);
}

void AConsoleSink::flush() {
    fflush(stdout);
}

AStringView AConsoleSink::name() const noexcept {
    return "console";
}
