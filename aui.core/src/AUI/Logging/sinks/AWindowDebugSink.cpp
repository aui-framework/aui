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

#include "AWindowDebugSink.h"
#include "detail/LogFormat.h"
#include <fmt/format.h>

#if AUI_PLATFORM_WIN
#include <windows.h>
#include <AUI/Common/AStringView.h>
#endif

AWindowDebugSink::AWindowDebugSink() {}

void AWindowDebugSink::write(const ALogMessage& message) {
#if AUI_PLATFORM_WIN
    auto timestamp = aui::detail::log::formatTimestamp(message.timestampMs);
    std::string windowMsg;
    if (message.message.empty()) {
        windowMsg = fmt::format("[{}][{}][{}]: {}",
                                timestamp, message.threadName,
                                aui::detail::log::levelCStr(message.level),
                                message.prefix);
    } else {
        windowMsg = fmt::format("[{}][{}][{}][{}]: {}",
                                timestamp, message.threadName, message.prefix,
                                aui::detail::log::levelCStr(message.level),
                                message.message);
    }
    std::wstring wideMessage = aui::win32::toWchar(AStringView(windowMsg));
    OutputDebugStringW(wideMessage.c_str());
    OutputDebugStringW(L"\n");
#else
    (void)message;
#endif
}

void AWindowDebugSink::flush() {}

AStringView AWindowDebugSink::name() const noexcept {
    return "windows_debug";
}
