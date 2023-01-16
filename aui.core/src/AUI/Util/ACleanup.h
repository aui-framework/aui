// AUI Framework - Declarative UI toolkit for modern C++17
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

#include <functional>
#include <AUI/Common/AObject.h>
#include <AUI/Common/AQueue.h>


/**
 * @brief Cleanup helper.
 * @ingroup core
 */
class API_AUI_CORE ACleanup {
    friend void afterEntryCleanup();
public:
    using Callback = std::function<void()>;

    /**
     * @param callback callback that called after AUI_ENTRY but before native entry point exit.
     */
    static void afterEntry(Callback callback) {
        std::unique_lock lock(inst().mSync);
        inst().mCallbacks << std::move(callback);
    }

private:
    static ACleanup& inst();

    void afterEntryPerform();

    AMutex mSync;
    AQueue<Callback> mCallbacks;
};


