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

#include <AUI/Platform/Entry.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/AMimedData.h>
#include "AUI/Common/AByteBuffer.h"

AUI_ENTRY {
    ALogger::info("Hello world!");
    AMimedData d;

    auto c = AString(AByteBuffer::fromStream(":test.txt"_url.open()));
    ALogger::info("test.txt") << c;
    return c == "test" ? 0 : -1;
}
