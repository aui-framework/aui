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

#if AUI_PROFILING

#include "AUI/Performance/APerformanceFrame.h"

APerformanceFrame::APerformanceFrame(Consumer consumer): mConsumer(std::move(consumer)) {
    AUI_ASSERTX(currentStorage() == nullptr, "there are could not be 2 APerformance frames at the same time");
    currentStorage() = this;
}

APerformanceFrame::~APerformanceFrame() {
    AUI_ASSERT(currentStorage() == this);
    currentStorage() = nullptr;

    mConsumer(std::move(mSections));
}

#endif