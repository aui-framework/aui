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

#include <AUI/Common/AObject.h>
#include <cassert>
#include <chrono>
#include "AUI/Common/AOptional.h"
#include "AUI/Thread/AMutex.h"
#include "AUI/Traits/concepts.h"
#include "AUI/Util/ARaiiHelper.h"
#include "AUI/Util/kAUI.h"

/**
 * @brief Watchdog helper class.
 * @ingroup core
 * @details
 * Watchdog is either hardware or software (as in this case) module which detects hangs of subject (in this case,
 * of a callback).
 *
 * AWatchdog catches ANR (Application Not Responsive) errors in GUI AUI applications.
 *
 * It's important to periodically call AWatchdog::isHang in order to check for hangs. It can be done via separate
 * thread or AUI's timers (separate thread is more reliable).
 *
 * @see AWatchdog::run
 * @see AWatchdog::isHang
 */
class API_AUI_CORE AWatchdog {
public:
    /**
     * @param hangDuration the time frame exceeding which considered as hang.
     */
    AWatchdog(std::chrono::high_resolution_clock::duration hangDuration = std::chrono::seconds(10)): mHangDuration(hangDuration) {}

    /**
     * @brief Runs callback which is subject to check for hangs.
     * @details
     * If execution time of operation exceeds mHangDuration, the operation is considered hang and AWatchdog::isHang
     * would report hang state when AWatchdog::isHang is called.
     */
    template<aui::invocable Operation>
    auto runOperation(Operation&& operation) -> decltype(operation()) {
        std::unique_lock lock(mSync);
        mBeginPoint = std::chrono::high_resolution_clock::now();
        AUI_DEFER {
            lock.lock();
            mBeginPoint.reset();
        };
        lock.unlock();
        return operation();
    }

    /**
     * @brief Checks for hang state.
     * @return true, if runOperation exceeded mHangDuration.
     */
    [[nodiscard]]
    bool isHang() const noexcept;
     
private:
    mutable AMutex mSync;
    std::chrono::high_resolution_clock::duration mHangDuration; 
    AOptional<std::chrono::high_resolution_clock::time_point> mBeginPoint;
};