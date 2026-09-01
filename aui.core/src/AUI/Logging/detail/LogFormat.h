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

#include <string>

namespace aui::detail::log {

    inline const char* levelCStr(int level) {
        switch (level) {
            case 0: return "INFO";
            case 1: return "WARN";
            case 2: return "ERR";
            case 3: return "DEBUG";
            case 4: return "TRACE";
        }
        return "UNKNOWN";
    }
}