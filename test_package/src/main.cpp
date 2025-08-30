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
#include <AUI/Common/AByteBuffer.h>
#include <AUI/Url/AUrl.h>

AUI_ENTRY {
    auto buf = AByteBuffer::fromStream(AUrl(":test.txt").open());
    return std::memcmp(buf.data(), "azaza", 5);
}
