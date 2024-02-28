// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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

#include <AUI/Thread/AMutex.h>
#include <AUI/Common/AQueue.h>

/**
 * @brief Thread-safe implementation of function queue.
 * @ingroup core
 */
class AFunctionQueue {
public:


    void operator<<(std::function<void()> function) {
        std::unique_lock lock(mLock);
        mQueue << std::move(function);
    }

    void process() {
        std::unique_lock l(mLock);
        while (!mQueue.empty()) {
            auto f = std::move(mQueue.front());
            mQueue.pop();
            l.unlock();
            f();
            l.lock();
        }
    }

    [[nodiscard]]
    bool empty() noexcept {
        std::unique_lock l(mLock);
        return mQueue.empty();
    }

private:
    AMutex mLock;
    AQueue<std::function<void()>> mQueue;
};