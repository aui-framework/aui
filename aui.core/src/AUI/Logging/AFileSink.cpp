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

#include "AFileSink.h"
#include "detail/LogFormat.h"
#include <fmt/format.h>
#include <cstdio>

AFileSink::AFileSink(APath path) : mFile(std::move(path)) {}

AFileSink::~AFileSink() {
    flush();
}

void AFileSink::write(const ALogMessage& message) {
    std::unique_lock lock(mSync);
    if (message.message.empty()) {
        fmt::println(mFile.nativeHandle(), "[{}][{}][{}]: {}",
                     message.timestamp, message.threadName,
                     aui::detail::log::levelCStr(message.level), message.prefix);
    } else {
        fmt::println(mFile.nativeHandle(), "[{}][{}][{}][{}]: {}",
                     message.timestamp, message.threadName, message.prefix,
                     aui::detail::log::levelCStr(message.level), message.message);
    }
    fflush(mFile.nativeHandle());
}

void AFileSink::flush() {
    std::unique_lock lock(mSync);
    fflush(mFile.nativeHandle());
}

std::string_view AFileSink::name() const noexcept {
    return "file";
}

APath AFileSink::path() const {
    return mFile.path();
}