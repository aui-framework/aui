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

#include <AUI/Core.h>
#include <string>

/**
 * @brief A single log record passed from an ALogger to its sinks.
 * @ingroup core
 */
struct ALogMessage {
    /// ALogger::Level enum value.
    int level = 0;

    /// Log prefix (e.g. tag / component name).
    std::string prefix;

    /// Raw log message body. May be empty, in which case `prefix` is the body.
    std::string message;

    /// Current thread name.
    std::string threadName;

    /// Timestamp formatted as "HH:MM:SS".
    std::string timestamp;
};

/**
 * @brief Abstract base class for log sinks.
 * @ingroup core
 * @details
 * A sink receives log records from an ALogger and writes them to a
 * destination (console, file, network, etc.). Each sink is responsible for
 * its own output formatting.
 *
 * Mirrors spdlog's sink concept.
 */
class API_AUI_CORE ALogSink {
public:
    virtual ~ALogSink() = default;

    /**
     * @brief Writes a log message.
     * @param message the log record
     */
    virtual void write(const ALogMessage& message) = 0;

    /**
     * @brief Flushes any buffered output.
     */
    virtual void flush() = 0;

    /**
     * @brief Returns a human-readable name for this sink.
     */
    virtual std::string_view name() const noexcept = 0;
};