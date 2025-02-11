/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2022 Alex2772
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <AUI/Platform/Entry.h>
#include <AUI/Logging/ALogger.h>

static constexpr auto LOG_TAG = "MyApp";

AUI_ENTRY {
    ALogger::info(LOG_TAG) << "Hello world!";
    return 0;
}