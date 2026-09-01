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
#include <AUI/IO/AFileOutputStream.h>
#include <AUI/IO/APath.h>
#include <AUI/Thread/AMutex.h>
#include <AUI/Util/ARaiiHelper.h>

/**
 * @brief File log sink.
 * @ingroup core
 * @details
 * Writes log output to a file. Thread-safe.
 * Mirrors spdlog's basic_file_sink.
 */
class API_AUI_CORE AFileSink : public ALogSink {
public:
    explicit AFileSink(APath path);
    ~AFileSink() override;

    void write(const ALogMessage& message) override;
    void flush() override;
    std::string_view name() const noexcept override;

    [[nodiscard]]
    APath path() const;

    /**
     * @brief Closes the file, executes an action, then reopens it.
     * @details Useful for reading the log file while it's being written to.
     */
    template <aui::invocable Callable>
    void doAccessSafe(Callable action) {
        std::unique_lock lock(mSync);
        ARaiiHelper opener = [&] {
            try {
                mFile.open(true);
            } catch (const AException& e) {
                auto p = mFile.path();
                mFile = AFileOutputStream(std::move(p));
                lock.unlock();
            }
        };
        if (!mFile.nativeHandle()) {
            action();
            return;
        }
        mFile.close();
        action();
    }

private:
    AFileOutputStream mFile;
    AMutex mSync;
};
