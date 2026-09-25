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
 * @brief Console (stdout) log sink for desktop platforms.
 * @ingroup core
 * @details
 * Writes log output to stdout with optional ANSI-colored levels.
 * Mirrors spdlog's stdout_color_sink.
 *
 * On Android/iOS the corresponding platform sinks are used by default instead.
 */
class API_AUI_CORE AConsoleSink : public ALogSink {
public:
    /**
     * @brief Constructs a console sink.
     * @param useColors whether to emit ANSI-colored output
     */
    explicit AConsoleSink(bool useColors = true);

    void write(const ALogMessage& message) override;
    void flush() override;
    AStringView name() const noexcept override;

    void setColorsEnabled(bool enabled) { mUseColors = enabled && !AUI_PLATFORM_WIN; }

private:
    bool mUseColors;
};
