// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <AUI/Common/AObject.h>
#include <cassert>
#include <chrono>
#include "AUI/Common/AOptional.h"
#include "AUI/Thread/AMutex.h"
#include "AUI/Traits/concepts.h"
#include "AUI/Util/ARaiiHelper.h"

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
        assert(!mBeginPoint.hasValue());
        mBeginPoint = std::chrono::high_resolution_clock::now();
        ARaiiHelper resetter = [&] {
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