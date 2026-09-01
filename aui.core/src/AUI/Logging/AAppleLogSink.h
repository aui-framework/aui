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

#include "ALogSink.h"
#include <AUI/Core.h>

/**
 * @brief Apple (iOS/macOS) log sink.
 * @ingroup core
 * @details
 * Routes log output to the unified system log via `NSLog`.
 * Mirrors spdlog's apple_sink.
 */
class API_AUI_CORE AAppleLogSink : public ALogSink {
public:
    AAppleLogSink();

    void write(const ALogMessage& message) override;
    void flush() override;
    std::string_view name() const noexcept override;
};