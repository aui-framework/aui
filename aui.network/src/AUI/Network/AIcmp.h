/*
 * AUI Framework - Declarative UI toolkit for modern C++20
 * Copyright (C) 2020-2024 Alex2772 and Contributors
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <AUI/Common/AObject.h>
#include "AUI/Thread/AFuture.h"
#include "AInet4Address.h"

namespace AIcmp {
    /**
     * @brief Sends ICMP ping echo packet.
     * @param destination destination address
     * @param timeout operation timeout
     * @return echo response result
     */
    API_AUI_NETWORK AFuture<std::chrono::high_resolution_clock::duration> ping(AInet4Address destination,
                                                                               std::chrono::milliseconds timeout = std::chrono::milliseconds(1000)) noexcept;
};
