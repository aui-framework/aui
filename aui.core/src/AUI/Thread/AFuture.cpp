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

#include "AFuture.h"
#include "AAsyncHolder.h"
#include "AUI/Common/ATimer.h"
#include "AUI/Util/AScheduler.h"

void aui::impl::future::scheduleTimeout(std::chrono::milliseconds duration, std::function<void()> callback) {
    ATimer::scheduler().enqueue(duration, std::move(callback));
}
