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

#include <cstdlib>
#include <string_view>

#include "ARemoteAgent.h"
#include "AUI/Thread/AThread.h"

namespace {
/// Starts the agent on the UI thread when the application is run with AUI_REMOTE=1. Static initialization happens on
/// the main thread, which is the UI thread, so AThread::main() is the right target; the task runs once the event loop
/// processes messages.
struct ARemoteAgentAutostart {
    ARemoteAgentAutostart() {
        const char* value = std::getenv("AUI_REMOTE");
        if (value == nullptr || std::string_view(value) != "1") {
            return;
        }
        AThread::main()->enqueue([] { ARemoteAgent::startIfRequested(); });
    }
} gAutostart;
}   // namespace
